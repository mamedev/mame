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

Priority encoder.  Always found in conjunction with k054338, but the reverse
isn't true.  The 55555 has 8 inputs: "A", "B", "C", and "D" intended for a 156/157
type tilemap chip, "OBJ" intended for a '246 type sprite chip, and "SUB1-SUB3"
which can be used for 3 additional layers.

When used in combintion with a k054338, each input can be chosen to participate
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


k054338
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

#include "emu.h"
#include "video/konicdev.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


#define XOR(a) WORD_XOR_BE(a)

/*
    This recursive function doesn't use additional memory
    (it could be easily converted into an iterative one).
    It's called shuffle because it mimics the shuffling of a deck of cards.
*/
static void konami_shuffle_16(UINT16 *buf,int len)
{
	int i;
	UINT16 t;

	if (len == 2) return;

	if (len % 4) fatalerror("shuffle() - not modulo 4");   /* must not happen */

	len /= 2;

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	konami_shuffle_16(buf,len);
	konami_shuffle_16(buf + len,len);
}

static void konami_shuffle_8(UINT8 *buf,int len)
{
	int i;
	UINT8 t;

	if (len == 2) return;

	if (len % 4) fatalerror("shuffle() - not modulo 4");	/* must not happen */

	len /= 2;

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	konami_shuffle_8(buf,len);
	konami_shuffle_8(buf + len,len);
}


/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konamid_rom_deinterleave_2(running_machine *machine, const char *mem_region)
{
	konami_shuffle_16((UINT16 *)memory_region(machine, mem_region),memory_region_length(machine, mem_region)/2);
}

/* hacked version of rom_deinterleave_2_half for Lethal Enforcers */
void konamid_rom_deinterleave_2_half(running_machine *machine, const char *mem_region)
{
	UINT8 *rgn = memory_region(machine, mem_region);

	konami_shuffle_16((UINT16 *)rgn,memory_region_length(machine, mem_region)/4);
	konami_shuffle_16((UINT16 *)(rgn+memory_region_length(machine, mem_region)/2),memory_region_length(machine, mem_region)/4);
}

/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konamid_rom_deinterleave_4(running_machine *machine, const char *mem_region)
{
	konamid_rom_deinterleave_2(machine, mem_region);
	konamid_rom_deinterleave_2(machine, mem_region);
}


static void decode_gfx(running_machine *machine, int gfx_index, UINT8 *data, UINT32 total, const gfx_layout *layout, int bpp)
{
	gfx_layout gl;

	memcpy(&gl, layout, sizeof(gl));
	gl.total = total;
	machine->gfx[gfx_index] = gfx_element_alloc(machine, &gl, data, machine->config->total_colors >> bpp, 0);
}


static void deinterleave_gfx(running_machine *machine, const char *gfx_memory_region, int deinterleave)
{
	switch (deinterleave)
	{
	case KONAMI_ROM_DEINTERLEAVE_NONE:
		break;
	case KONAMI_ROM_DEINTERLEAVE_2:
		konamid_rom_deinterleave_2(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_DEINTERLEAVE_2_HALF:
		konamid_rom_deinterleave_2_half(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_DEINTERLEAVE_4:
		konamid_rom_deinterleave_4(machine, gfx_memory_region);
		break;
	case KONAMI_ROM_SHUFFLE8:
		konami_shuffle_8(memory_region(machine, gfx_memory_region), memory_region_length(machine, gfx_memory_region));
		break;
	}
}

/* useful function to sort three tile layers by priority order */
void konami_sortlayers3( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
#undef  SWAP
}

/* useful function to sort four tile layers by priority order */
void konami_sortlayers4( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] <= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(2, 3)
#undef  SWAP
}

/* useful function to sort five tile layers by priority order */
void konami_sortlayers5( int *layer, int *pri )
{
#define SWAP(a,b) \
	if (pri[a] <= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0, 1)
	SWAP(0, 2)
	SWAP(0, 3)
	SWAP(0, 4)
	SWAP(1, 2)
	SWAP(1, 3)
	SWAP(1, 4)
	SWAP(2, 3)
	SWAP(2, 4)
	SWAP(3, 4)
#undef  SWAP
}


/***************************************************************************/
/*                                                                         */
/*                                 007121                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k007121_state  k007121_state ;
struct _k007121_state
{

	UINT8    ctrlram[8];
	int      flipscreen;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k007121_state *k007121_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K007121);

	return (k007121_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* FIXME: this is probably unused... check! */
WRITE8_DEVICE_HANDLER( k007121_ctrlram_w )
{
	k007121_state *k007121 = k007121_get_safe_token(device);

	assert(offset < 8);

	k007121->ctrlram[offset] = data;
}


READ8_DEVICE_HANDLER( k007121_ctrlram_r )
{
	k007121_state *k007121 = k007121_get_safe_token(device);

	assert(offset < 8);

	return k007121->ctrlram[offset];
}


WRITE8_DEVICE_HANDLER( k007121_ctrl_w )
{
	k007121_state *k007121 = k007121_get_safe_token(device);

	assert(offset < 8);

	switch (offset)
	{
	case 6:
		/* palette bank change */
		if ((k007121->ctrlram[offset] & 0x30) != (data & 0x30))
			tilemap_mark_all_tiles_dirty_all(device->machine);
		break;
	case 7:
		k007121->flipscreen = data & 0x08;
		break;
	}

	k007121->ctrlram[offset] = data;
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

void k007121_sprites_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, gfx_element *gfx, colortable_t *ctable,
						  const UINT8 *source, int base_color, int global_x_offset, int bank_base, UINT32 pri_mask )
{
	k007121_state *k007121 = k007121_get_safe_token(device);
//  const gfx_element *gfx = gfxs[chip];
	bitmap_t *priority_bitmap = gfx->machine->priority_bitmap;
	int flipscreen = k007121->flipscreen;
	int i, num, inc, offs[5];
	int is_flakatck = (ctable == NULL);

	if (is_flakatck)
	{
		num = 0x40;
		inc = -0x20;
		source += 0x3f * 0x20;
		offs[0] = 0x0e;
		offs[1] = 0x0f;
		offs[2] = 0x06;
		offs[3] = 0x04;
		offs[4] = 0x08;
	}
	else	/* all others */
	{
		//num = (k007121->ctrlram[0x03] & 0x40) ? 0x80 : 0x40; /* WRONG!!! (needed by combasc)  */
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
			source += (num - 1)*inc;
			inc = -inc;
		}
	}

	for (i = 0; i < num; i++)
	{
		int number = source[offs[0]];				/* sprite number */
		int sprite_bank = source[offs[1]] & 0x0f;	/* sprite bank */
		int sx = source[offs[3]];					/* vertical position */
		int sy = source[offs[2]];					/* horizontal position */
		int attr = source[offs[4]];				/* attributes */
		int xflip = source[offs[4]] & 0x10;		/* flip x */
		int yflip = source[offs[4]] & 0x20;		/* flip y */
		int color = base_color + ((source[offs[1]] & 0xf0) >> 4);
		int width, height;
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

			switch (attr & 0xe)
			{
				case 0x06: width = height = 1; break;
				case 0x04: width = 1; height = 2; number &= (~2); break;
				case 0x02: width = 2; height = 1; number &= (~1); break;
				case 0x00: width = height = 2; number &= (~3); break;
				case 0x08: width = height = 4; number &= (~3); break;
				default: width = 1; height = 1;
//                  logerror("Unknown sprite size %02x\n", attr & 0xe);
//                  popmessage("Unknown sprite size %02x\n", attr & 0xe);
			}

			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					ex = xflip ? (width - 1 - x) : x;
					ey = yflip ? (height - 1 - y) : y;

					if (flipscreen)
					{
						flipx = !xflip;
						flipy = !yflip;
						destx = 248 - (sx + x * 8);
						desty = 248 - (sy + y * 8);
					}
					else
					{
						flipx = xflip;
						flipy = yflip;
						destx = global_x_offset + sx + x * 8;
						desty = sy + y * 8;
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

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k007121 )
{
	k007121_state *k007121 = k007121_get_safe_token(device);

	state_save_register_device_item_array(device, 0, k007121->ctrlram);
	state_save_register_device_item(device, 0, k007121->flipscreen);
}

static DEVICE_RESET( k007121 )
{
	k007121_state *k007121 = k007121_get_safe_token(device);
	int i;

	k007121->flipscreen = 0;

	for (i = 0; i < 8; i++)
		k007121->ctrlram[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                                 007342                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k007342_state k007342_state;
struct _k007342_state
{
	UINT8    *ram;
	UINT8    *scroll_ram;
	UINT8    *videoram_0;
	UINT8    *videoram_1;
	UINT8    *colorram_0;
	UINT8    *colorram_1;

	tilemap_t  *tilemap[2];
	int      flipscreen, gfxnum, int_enabled;
	UINT8    regs[8];
	UINT16   scrollx[2];
	UINT8    scrolly[2];

	k007342_callback callback;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k007342_state *k007342_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K007342);

	return (k007342_state *)device->token;
}

INLINE const k007342_interface *k007342_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K007342));
	return (const k007342_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k007342_r )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	return k007342->ram[offset];
}

WRITE8_DEVICE_HANDLER( k007342_w )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	k007342->ram[offset] = data;

	if (offset < 0x1000)	/* layer 0 */
		tilemap_mark_tile_dirty(k007342->tilemap[0], offset & 0x7ff);
	else				/* layer 1 */
		tilemap_mark_tile_dirty(k007342->tilemap[1], offset & 0x7ff);
}

READ8_DEVICE_HANDLER( k007342_scroll_r )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	return k007342->scroll_ram[offset];
}

WRITE8_DEVICE_HANDLER( k007342_scroll_w )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	k007342->scroll_ram[offset] = data;
}

WRITE8_DEVICE_HANDLER( k007342_vreg_w )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	switch(offset)
	{
		case 0x00:
			/* bit 1: INT control */
			k007342->int_enabled = data & 0x02;
			k007342->flipscreen = data & 0x10;
			tilemap_set_flip(k007342->tilemap[0], k007342->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(k007342->tilemap[1], k007342->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			break;
		case 0x01:  /* used for banking in Rock'n'Rage */
			if (data != k007342->regs[1])
				tilemap_mark_all_tiles_dirty_all(device->machine);
		case 0x02:
			k007342->scrollx[0] = (k007342->scrollx[0] & 0xff) | ((data & 0x01) << 8);
			k007342->scrollx[1] = (k007342->scrollx[1] & 0xff) | ((data & 0x02) << 7);
			break;
		case 0x03:  /* scroll x (register 0) */
			k007342->scrollx[0] = (k007342->scrollx[0] & 0x100) | data;
			break;
		case 0x04:  /* scroll y (register 0) */
			k007342->scrolly[0] = data;
			break;
		case 0x05:  /* scroll x (register 1) */
			k007342->scrollx[1] = (k007342->scrollx[1] & 0x100) | data;
			break;
		case 0x06:  /* scroll y (register 1) */
			k007342->scrolly[1] = data;
		case 0x07:  /* unused */
			break;
	}
	k007342->regs[offset] = data;
}

void k007342_tilemap_update( const device_config *device )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	int offs;

	/* update scroll */
	switch (k007342->regs[2] & 0x1c)
	{
		case 0x00:
		case 0x08:	/* unknown, blades of steel shootout between periods */
			tilemap_set_scroll_rows(k007342->tilemap[0], 1);
			tilemap_set_scroll_cols(k007342->tilemap[0], 1);
			tilemap_set_scrollx(k007342->tilemap[0], 0, k007342->scrollx[0]);
			tilemap_set_scrolly(k007342->tilemap[0], 0, k007342->scrolly[0]);
			break;

		case 0x0c:	/* 32 columns */
			tilemap_set_scroll_rows(k007342->tilemap[0], 1);
			tilemap_set_scroll_cols(k007342->tilemap[0], 512);
			tilemap_set_scrollx(k007342->tilemap[0], 0, k007342->scrollx[0]);
			for (offs = 0; offs < 256; offs++)
				tilemap_set_scrolly(k007342->tilemap[0], (offs + k007342->scrollx[0]) & 0x1ff,
						k007342->scroll_ram[2 * (offs / 8)] + 256 * k007342->scroll_ram[2 * (offs / 8) + 1]);
			break;

		case 0x14:	/* 256 rows */
			tilemap_set_scroll_rows(k007342->tilemap[0], 256);
			tilemap_set_scroll_cols(k007342->tilemap[0], 1);
			tilemap_set_scrolly(k007342->tilemap[0], 0, k007342->scrolly[0]);
			for (offs = 0; offs < 256; offs++)
				tilemap_set_scrollx(k007342->tilemap[0], (offs + k007342->scrolly[0]) & 0xff,
						k007342->scroll_ram[2 * offs] + 256 * k007342->scroll_ram[2 * offs + 1]);
			break;

		default:
//          popmessage("unknown scroll ctrl %02x", k007342->regs[2] & 0x1c);
			break;
	}

	tilemap_set_scrollx(k007342->tilemap[1], 0, k007342->scrollx[1]);
	tilemap_set_scrolly(k007342->tilemap[1], 0, k007342->scrolly[1]);

#if 0
	{
		static int current_layer = 0;

		if (input_code_pressed_once(machine, KEYCODE_Z)) current_layer = !current_layer;
		tilemap_set_enable(k007342->tilemap[current_layer], 1);
		tilemap_set_enable(k007342->tilemap[!current_layer], 0);

		popmessage("regs:%02x %02x %02x %02x-%02x %02x %02x %02x:%02x",
			k007342->regs[0], k007342->regs[1], k007342->regs[2], k007342->regs[3],
			k007342->regs[4], k007342->regs[5], k007342->regs[6], k007342->regs[7],
			current_layer);
	}
#endif
}

void k007342_tilemap_set_enable( const device_config *device, int tmap, int enable )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	tilemap_set_enable(k007342->tilemap[tmap], enable);
}

void k007342_tilemap_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int num, int flags, UINT32 priority )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	tilemap_draw(bitmap, cliprect, k007342->tilemap[num], flags, priority);
}

int k007342_is_int_enabled( const device_config *device )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	return k007342->int_enabled;
}


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

static TILEMAP_MAPPER( k007342_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

INLINE void k007342_get_tile_info( const device_config *device, tile_data *tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	int color, code, flags;

	color = cram[tile_index];
	code = vram[tile_index];
	flags = TILE_FLIPYX((color & 0x30) >> 4);

	tileinfo->category = (color & 0x80) >> 7;

	k007342->callback(device->machine, layer, k007342->regs[1], &code, &color, &flags);

	SET_TILE_INFO_DEVICE(
			k007342->gfxnum,
			code,
			color,
			flags);
}

static TILE_GET_INFO_DEVICE( k007342_get_tile_info0 )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	k007342_get_tile_info(device, tileinfo, tile_index, 0, k007342->colorram_0, k007342->videoram_0);
}

static TILE_GET_INFO_DEVICE( k007342_get_tile_info1 )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	k007342_get_tile_info(device, tileinfo, tile_index, 1, k007342->colorram_1, k007342->videoram_1);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k007342 )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	const k007342_interface *intf = k007342_get_interface(device);

	k007342->gfxnum = intf->gfxnum;
	k007342->callback = intf->callback;

	k007342->tilemap[0] = tilemap_create_device(device, k007342_get_tile_info0, k007342_scan, 8, 8, 64, 32);
	k007342->tilemap[1] = tilemap_create_device(device, k007342_get_tile_info1, k007342_scan, 8, 8, 64, 32);

	k007342->ram = auto_alloc_array(device->machine, UINT8, 0x2000);
	k007342->scroll_ram = auto_alloc_array(device->machine, UINT8, 0x0200);

	k007342->colorram_0 = &k007342->ram[0x0000];
	k007342->colorram_1 = &k007342->ram[0x1000];
	k007342->videoram_0 = &k007342->ram[0x0800];
	k007342->videoram_1 = &k007342->ram[0x1800];

	tilemap_set_transparent_pen(k007342->tilemap[0], 0);
	tilemap_set_transparent_pen(k007342->tilemap[1], 0);

	state_save_register_device_item_pointer(device, 0, k007342->ram, 0x2000);
	state_save_register_device_item_pointer(device, 0, k007342->scroll_ram, 0x0200);
	state_save_register_device_item(device, 0, k007342->int_enabled);
	state_save_register_device_item(device, 0, k007342->flipscreen);
	state_save_register_device_item_array(device, 0, k007342->scrollx);
	state_save_register_device_item_array(device, 0, k007342->scrolly);
	state_save_register_device_item_array(device, 0, k007342->regs);
}

static DEVICE_RESET( k007342 )
{
	k007342_state *k007342 = k007342_get_safe_token(device);
	int i;

	k007342->int_enabled = 0;
	k007342->flipscreen = 0;
	k007342->scrollx[0] = 0;
	k007342->scrollx[1] = 0;
	k007342->scrolly[0] = 0;
	k007342->scrolly[1] = 0;

	for (i = 0; i < 8; i++)
		k007342->regs[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                                 007420                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k007420_state k007420_state;
struct _k007420_state
{
	UINT8        *ram;

	int          banklimit;
	int          flipscreen;	// current code uses the 7342 flipscreen!!
	UINT8        regs[8];	// current code uses the 7342 regs!! (only [2])

	k007420_callback callback;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k007420_state *k007420_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K007420);

	return (k007420_state *)device->token;
}

INLINE const k007420_interface *k007420_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K007420));
	return (const k007420_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k007420_r )
{
	k007420_state *k007420 = k007420_get_safe_token(device);
	return k007420->ram[offset];
}

WRITE8_DEVICE_HANDLER( k007420_w )
{
	k007420_state *k007420 = k007420_get_safe_token(device);
	k007420->ram[offset] = data;
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

void k007420_sprites_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, gfx_element *gfx )
{
	k007420_state *k007420 = k007420_get_safe_token(device);
	int offs;
	int codemask = k007420->banklimit;
	int bankmask = ~k007420->banklimit;

	for (offs = K007420_SPRITERAM_SIZE - 8; offs >= 0; offs -= 8)
	{
		int ox, oy, code, color, flipx, flipy, zoom, w, h, x, y, bank;
		static const int xoffset[4] = { 0, 1, 4, 5 };
		static const int yoffset[4] = { 0, 2, 8, 10 };

		code = k007420->ram[offs + 1];
		color = k007420->ram[offs + 2];
		ox = k007420->ram[offs + 3] - ((k007420->ram[offs + 4] & 0x80) << 1);
		oy = 256 - k007420->ram[offs + 0];
		flipx = k007420->ram[offs + 4] & 0x04;
		flipy = k007420->ram[offs + 4] & 0x08;

		k007420->callback(device->machine, &code, &color);

		bank = code & bankmask;
		code &= codemask;

		/* 0x080 = normal scale, 0x040 = double size, 0x100 half size */
		zoom = k007420->ram[offs + 5] | ((k007420->ram[offs + 4] & 0x03) << 8);
		if (!zoom)
			continue;
		zoom = 0x10000 * 128 / zoom;

		switch (k007420->ram[offs + 4] & 0x70)
		{
			case 0x30: w = h = 1; break;
			case 0x20: w = 2; h = 1; code &= (~1); break;
			case 0x10: w = 1; h = 2; code &= (~2); break;
			case 0x00: w = h = 2; code &= (~3); break;
			case 0x40: w = h = 4; code &= (~3); break;
			default: w = 1; h = 1;
//logerror("Unknown sprite size %02x\n",(k007420->ram[offs + 4] & 0x70) >> 4);
		}

		if (k007420->flipscreen)
		{
			ox = 256 - ox - ((zoom * w + (1 << 12)) >> 13);
			oy = 256 - oy - ((zoom * h + (1 << 12)) >> 13);
			flipx = !flipx;
			flipy = !flipy;
		}

		if (zoom == 0x10000)
		{
			int sx, sy;

			for (y = 0; y < h; y++)
			{
				sy = oy + 8 * y;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + 8 * x;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					drawgfx_transpen(bitmap,cliprect,gfx,
						c,
						color,
						flipx,flipy,
						sx,sy,0);

					if (k007420->regs[2] & 0x80)
						drawgfx_transpen(bitmap,cliprect,gfx,
							c,
							color,
							flipx,flipy,
							sx,sy-256,0);
				}
			}
		}
		else
		{
			int sx, sy, zw, zh;
			for (y = 0; y < h; y++)
			{
				sy = oy + ((zoom * y + (1 << 12)) >> 13);
				zh = (oy + ((zoom * (y + 1) + (1 << 12)) >> 13)) - sy;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + ((zoom * x + (1<<12)) >> 13);
					zw = (ox + ((zoom * (x + 1) + (1 << 12)) >> 13)) - sx;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					drawgfxzoom_transpen(bitmap,cliprect,gfx,
						c,
						color,
						flipx,flipy,
						sx,sy,
						(zw << 16) / 8,(zh << 16) / 8,0);

					if (k007420->regs[2] & 0x80)
						drawgfxzoom_transpen(bitmap,cliprect,gfx,
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
			k007420->ram[(current_sprite*8)+0], k007420->ram[(current_sprite*8)+1],
			k007420->ram[(current_sprite*8)+2], k007420->ram[(current_sprite*8)+3],
			k007420->ram[(current_sprite*8)+4], k007420->ram[(current_sprite*8)+5],
			k007420->ram[(current_sprite*8)+6], k007420->ram[(current_sprite*8)+7]);
	}
#endif
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k007420 )
{
	k007420_state *k007420 = k007420_get_safe_token(device);
	const k007420_interface *intf = k007420_get_interface(device);

	k007420->callback = intf->callback;
	k007420->banklimit = intf->banklimit;

	k007420->ram = auto_alloc_array(device->machine, UINT8, 0x200);

	state_save_register_device_item_pointer(device, 0, k007420->ram, 0x200);
	state_save_register_device_item(device, 0, k007420->flipscreen);	// current one uses 7342 one
	state_save_register_device_item_array(device, 0, k007420->regs);	// current one uses 7342 ones
}

static DEVICE_RESET( k007420 )
{
	k007420_state *k007420 = k007420_get_safe_token(device);
	int i;

	k007420->flipscreen = 0;
	for (i = 0; i < 8; i++)
		k007420->regs[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                                 052109                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k052109_state k052109_state;
struct _k052109_state
{
	UINT8    *ram;
	UINT8    *videoram_F;
	UINT8    *videoram_A;
	UINT8    *videoram_B;
	UINT8    *videoram2_F;
	UINT8    *videoram2_A;
	UINT8    *videoram2_B;
	UINT8    *colorram_F;
	UINT8    *colorram_A;
	UINT8    *colorram_B;

	tilemap_t  *tilemap[3];
	int      tileflip_enable, gfxnum;
	UINT8    charrombank[4];
	UINT8    charrombank_2[4];
	UINT8    has_extra_video_ram;
	INT32    rmrd_line;
	UINT8    irq_enabled;
	INT32    dx[3], dy[3];
	UINT8    romsubbank, scrollctrl;

	k052109_callback callback;

	const char *memory_region;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k052109_state *k052109_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K052109);

	return (k052109_state *)device->token;
}

INLINE const k052109_interface *k052109_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K052109));
	return (const k052109_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k052109_r )
{
	k052109_state *k052109 = k052109_get_safe_token(device);

	if (k052109->rmrd_line == CLEAR_LINE)
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

		return k052109->ram[offset];
	}
	else	/* Punk Shot and TMNT read from 0000-1fff, Aliens from 2000-3fff */
	{
		int code = (offset & 0x1fff) >> 5;
		int color = k052109->romsubbank;
		int flags = 0;
		int priority = 0;
		int bank = k052109->charrombank[(color & 0x0c) >> 2] >> 2;   /* discard low bits (TMNT) */
		int addr;

		bank |= (k052109->charrombank_2[(color & 0x0c) >> 2] >> 2); // Surprise Attack uses this 2nd bank in the rom test

	if (k052109->has_extra_video_ram)
		code |= color << 8;	/* kludge for X-Men */
	else
		k052109->callback(device->machine, 0, bank, &code, &color, &flags, &priority);

		addr = (code << 5) + (offset & 0x1f);
		addr &= memory_region_length(device->machine, k052109->memory_region) - 1;

//      logerror("%04x: off = %04x sub = %02x (bnk = %x) adr = %06x\n", cpu_get_pc(space->cpu), offset, k052109->romsubbank, bank, addr);

		return memory_region(device->machine, k052109->memory_region)[addr];
	}
}

WRITE8_DEVICE_HANDLER( k052109_w )
{
	k052109_state *k052109 = k052109_get_safe_token(device);

	if ((offset & 0x1fff) < 0x1800) /* tilemap RAM */
	{
		if (offset >= 0x4000)
			k052109->has_extra_video_ram = 1;  /* kludge for X-Men */

		k052109->ram[offset] = data;
		tilemap_mark_tile_dirty(k052109->tilemap[(offset & 0x1800) >> 11],offset & 0x7ff);
	}
	else	/* control registers */
	{
		k052109->ram[offset] = data;

		if (offset >= 0x180c && offset < 0x1834)
		{	/* A y scroll */	}
		else if (offset >= 0x1a00 && offset < 0x1c00)
		{	/* A x scroll */	}
		else if (offset == 0x1c80)
		{
			if (k052109->scrollctrl != data)
			{
//popmessage("scrollcontrol = %02x", data);
//logerror("%04x: rowscrollcontrol = %02x\n", cpu_get_pc(space->cpu), data);
				k052109->scrollctrl = data;
			}
		}
		else if (offset == 0x1d00)
		{
//logerror("%04x: 052109 register 1d00 = %02x\n", cpu_get_pc(space->cpu), data);
			/* bit 2 = irq enable */
			/* the custom chip can also generate NMI and FIRQ, for use with a 6809 */
			k052109->irq_enabled = data & 0x04;
		}
		else if (offset == 0x1d80)
		{
			int dirty = 0;

			if (k052109->charrombank[0] != (data & 0x0f))
				dirty |= 1;
			if (k052109->charrombank[1] != ((data >> 4) & 0x0f))
				dirty |= 2;

			if (dirty)
			{
				int i;

				k052109->charrombank[0] = data & 0x0f;
				k052109->charrombank[1] = (data >> 4) & 0x0f;

				for (i = 0; i < 0x1800; i++)
				{
					int bank = (k052109->ram[i]&0x0c) >> 2;
					if ((bank == 0 && (dirty & 1)) || (bank == 1 && (dirty & 2)))
					{
						tilemap_mark_tile_dirty(k052109->tilemap[(i & 0x1800) >> 11], i & 0x7ff);
					}
				}
			}
		}
		else if (offset == 0x1e00 || offset == 0x3e00) // Surprise Attack uses offset 0x3e00
		{
//logerror("%04x: 052109 register 1e00 = %02x\n",cpu_get_pc(space->cpu),data);
			k052109->romsubbank = data;
		}
		else if (offset == 0x1e80)
		{
//if ((data & 0xfe)) logerror("%04x: 052109 register 1e80 = %02x\n",cpu_get_pc(space->cpu),data);
			tilemap_set_flip(k052109->tilemap[0], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(k052109->tilemap[1], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(k052109->tilemap[2], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			if (k052109->tileflip_enable != ((data & 0x06) >> 1))
			{
				k052109->tileflip_enable = ((data & 0x06) >> 1);

				tilemap_mark_all_tiles_dirty(k052109->tilemap[0]);
				tilemap_mark_all_tiles_dirty(k052109->tilemap[1]);
				tilemap_mark_all_tiles_dirty(k052109->tilemap[2]);
			}
		}
		else if (offset == 0x1f00)
		{
			int dirty = 0;

			if (k052109->charrombank[2] != (data & 0x0f))
				dirty |= 1;

			if (k052109->charrombank[3] != ((data >> 4) & 0x0f))
				dirty |= 2;

			if (dirty)
			{
				int i;

				k052109->charrombank[2] = data & 0x0f;
				k052109->charrombank[3] = (data >> 4) & 0x0f;

				for (i = 0; i < 0x1800; i++)
				{
					int bank = (k052109->ram[i] & 0x0c) >> 2;
					if ((bank == 2 && (dirty & 1)) || (bank == 3 && (dirty & 2)))
						tilemap_mark_tile_dirty(k052109->tilemap[(i & 0x1800) >> 11],i & 0x7ff);
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
			k052109->charrombank_2[0] = data & 0x0f;
			k052109->charrombank_2[1] = (data >> 4) & 0x0f;
		}
		else if (offset == 0x3f00) // Surprise Attack uses offset 0x3f00 in rom test
		{
			// mirroring this write, breaks Surprise Attack in game tilemaps
			k052109->charrombank_2[2] = data & 0x0f;
			k052109->charrombank_2[3] = (data >> 4) & 0x0f;
		}
//      else
//          logerror("%04x: write %02x to unknown 052109 address %04x\n",cpu_get_pc(space->cpu),data,offset);
	}
}

READ16_DEVICE_HANDLER( k052109_word_r )
{
	return k052109_r(device, offset + 0x2000) | (k052109_r(device, offset) << 8);
}

WRITE16_DEVICE_HANDLER( k052109_word_w )
{
	if (ACCESSING_BITS_8_15)
		k052109_w(device, offset, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k052109_w(device, offset + 0x2000, data & 0xff);
}

READ16_DEVICE_HANDLER( k052109_lsb_r )
{
	return k052109_r(device, offset);
}

WRITE16_DEVICE_HANDLER( k052109_lsb_w )
{
	if(ACCESSING_BITS_0_7)
		k052109_w(device, offset, data & 0xff);
}

void k052109_set_rmrd_line( const device_config *device, int state )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	k052109->rmrd_line = state;
}

int k052109_get_rmrd_line(const device_config *device )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	return k052109->rmrd_line;
}


void k052109_tilemap_mark_dirty( const device_config *device, int tmap_num )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	tilemap_mark_all_tiles_dirty(k052109->tilemap[tmap_num]);
}


void k052109_tilemap_update( const device_config *device )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	int xscroll, yscroll, offs;

#if 0
{
popmessage("%x %x %x %x",
	k052109->charrombank[0],
	k052109->charrombank[1],
	k052109->charrombank[2],
	k052109->charrombank[3]);
}
#endif

	if ((k052109->scrollctrl & 0x03) == 0x02)
	{
		UINT8 *scrollram = &k052109->ram[0x1a00];

		tilemap_set_scroll_rows(k052109->tilemap[1], 256);
		tilemap_set_scroll_cols(k052109->tilemap[1], 1);
		yscroll = k052109->ram[0x180c];
		tilemap_set_scrolly(k052109->tilemap[1], 0, yscroll + k052109->dy[1]);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * (offs & 0xfff8) + 0] + 256 * scrollram[2 * (offs & 0xfff8) + 1];
			xscroll -= 6;
			tilemap_set_scrollx(k052109->tilemap[1], (offs + yscroll) & 0xff, xscroll + k052109->dx[1]);
		}
	}
	else if ((k052109->scrollctrl & 0x03) == 0x03)
	{
		UINT8 *scrollram = &k052109->ram[0x1a00];

		tilemap_set_scroll_rows(k052109->tilemap[1], 256);
		tilemap_set_scroll_cols(k052109->tilemap[1], 1);
		yscroll = k052109->ram[0x180c];
		tilemap_set_scrolly(k052109->tilemap[1], 0, yscroll + k052109->dy[1]);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			xscroll -= 6;
			tilemap_set_scrollx(k052109->tilemap[1], (offs + yscroll) & 0xff, xscroll + k052109->dx[1]);
		}
	}
	else if ((k052109->scrollctrl & 0x04) == 0x04)
	{
		UINT8 *scrollram = &k052109->ram[0x1800];

		tilemap_set_scroll_rows(k052109->tilemap[1], 1);
		tilemap_set_scroll_cols(k052109->tilemap[1], 512);
		xscroll = k052109->ram[0x1a00] + 256 * k052109->ram[0x1a01];
		xscroll -= 6;
		tilemap_set_scrollx(k052109->tilemap[1], 0, xscroll + k052109->dx[1]);
		for (offs = 0; offs < 512; offs++)
		{
			yscroll = scrollram[offs / 8];
			tilemap_set_scrolly(k052109->tilemap[1], (offs + xscroll) & 0x1ff, yscroll + k052109->dy[1]);
		}
	}
	else
	{
		UINT8 *scrollram = &k052109->ram[0x1a00];

		tilemap_set_scroll_rows(k052109->tilemap[1], 1);
		tilemap_set_scroll_cols(k052109->tilemap[1], 1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = k052109->ram[0x180c];
		tilemap_set_scrollx(k052109->tilemap[1], 0, xscroll + k052109->dx[1]);
		tilemap_set_scrolly(k052109->tilemap[1], 0, yscroll + k052109->dy[1]);
	}

	if ((k052109->scrollctrl & 0x18) == 0x10)
	{
		UINT8 *scrollram = &k052109->ram[0x3a00];

		tilemap_set_scroll_rows(k052109->tilemap[2], 256);
		tilemap_set_scroll_cols(k052109->tilemap[2], 1);
		yscroll = k052109->ram[0x380c];
		tilemap_set_scrolly(k052109->tilemap[2], 0, yscroll + k052109->dy[2]);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * (offs & 0xfff8) + 0] + 256 * scrollram[2 * (offs & 0xfff8) + 1];
			xscroll -= 6;
			tilemap_set_scrollx(k052109->tilemap[2], (offs + yscroll) & 0xff, xscroll + k052109->dx[2]);
		}
	}
	else if ((k052109->scrollctrl & 0x18) == 0x18)
	{
		UINT8 *scrollram = &k052109->ram[0x3a00];

		tilemap_set_scroll_rows(k052109->tilemap[2], 256);
		tilemap_set_scroll_cols(k052109->tilemap[2], 1);
		yscroll = k052109->ram[0x380c];
		tilemap_set_scrolly(k052109->tilemap[2], 0, yscroll + k052109->dy[2]);
		for (offs = 0; offs < 256; offs++)
		{
			xscroll = scrollram[2 * offs + 0] + 256 * scrollram[2 * offs + 1];
			xscroll -= 6;
			tilemap_set_scrollx(k052109->tilemap[2], (offs + yscroll) & 0xff, xscroll + k052109->dx[2]);
		}
	}
	else if ((k052109->scrollctrl & 0x20) == 0x20)
	{
		UINT8 *scrollram = &k052109->ram[0x3800];

		tilemap_set_scroll_rows(k052109->tilemap[2], 1);
		tilemap_set_scroll_cols(k052109->tilemap[2], 512);
		xscroll = k052109->ram[0x3a00] + 256 * k052109->ram[0x3a01];
		xscroll -= 6;
		tilemap_set_scrollx(k052109->tilemap[2], 0, xscroll + k052109->dx[2]);
		for (offs = 0; offs < 512; offs++)
		{
			yscroll = scrollram[offs / 8];
			tilemap_set_scrolly(k052109->tilemap[2], (offs + xscroll) & 0x1ff, yscroll + k052109->dy[2]);
		}
	}
	else
	{
		UINT8 *scrollram = &k052109->ram[0x3a00];

		tilemap_set_scroll_rows(k052109->tilemap[2], 1);
		tilemap_set_scroll_cols(k052109->tilemap[2], 1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = k052109->ram[0x380c];
		tilemap_set_scrollx(k052109->tilemap[2], 0, xscroll + k052109->dx[2]);
		tilemap_set_scrolly(k052109->tilemap[2], 0, yscroll + k052109->dy[2]);
	}

#if 0
if ((k052109->scrollctrl & 0x03) == 0x01 ||
		(k052109->scrollctrl & 0x18) == 0x08 ||
		((k052109->scrollctrl & 0x04) && (k052109->scrollctrl & 0x03)) ||
		((k052109->scrollctrl & 0x20) && (k052109->scrollctrl & 0x18)) ||
		(k052109->scrollctrl & 0xc0) != 0)
	popmessage("scrollcontrol = %02x", k052109->scrollctrl);

if (input_code_pressed(machine, KEYCODE_F))
{
	FILE *fp;
	fp=fopen("TILE.DMP", "w+b");
	if (fp)
	{
		fwrite(k052109->ram, 0x6000, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
}

void k052109_tilemap_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int tmap_num, UINT32 flags, UINT8 priority )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	tilemap_draw(bitmap, cliprect, k052109->tilemap[tmap_num], flags, priority);
}

int k052109_is_irq_enabled( const device_config *device )
{
	k052109_state *k052109 = k052109_get_safe_token(device);

	return k052109->irq_enabled;
}

void k052109_set_layer_offsets( const device_config *device, int layer, int dx, int dy )
{
	k052109_state *k052109 = k052109_get_safe_token(device);

	k052109->dx[layer] = dx;
	k052109->dy[layer] = dy;
}


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

INLINE void k052109_get_tile_info( const device_config *device, tile_data *tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram1, UINT8 *vram2 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	int flipy = 0;
	int code = vram1[tile_index] + 256 * vram2[tile_index];
	int color = cram[tile_index];
	int flags = 0;
	int priority = 0;
	int bank = k052109->charrombank[(color & 0x0c) >> 2];
	if (k052109->has_extra_video_ram)
		bank = (color & 0x0c) >> 2;	/* kludge for X-Men */

	color = (color & 0xf3) | ((bank & 0x03) << 2);
	bank >>= 2;

	flipy = color & 0x02;

	k052109->callback(device->machine, layer, bank, &code, &color, &flags, &priority);

	/* if the callback set flip X but it is not enabled, turn it off */
	if (!(k052109->tileflip_enable & 1))
		flags &= ~TILE_FLIPX;

	/* if flip Y is enabled and the attribute but is set, turn it on */
	if (flipy && (k052109->tileflip_enable & 2))
		flags |= TILE_FLIPY;

	SET_TILE_INFO_DEVICE(
			k052109->gfxnum,
			code,
			color,
			flags);

	tileinfo->category = priority;
}

static TILE_GET_INFO_DEVICE( k052109_get_tile_info0 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	k052109_get_tile_info(device, tileinfo, tile_index, 0, k052109->colorram_F, k052109->videoram_F, k052109->videoram2_F);
}

static TILE_GET_INFO_DEVICE( k052109_get_tile_info1 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	k052109_get_tile_info(device, tileinfo, tile_index, 1, k052109->colorram_A, k052109->videoram_A, k052109->videoram2_A);
}

static TILE_GET_INFO_DEVICE( k052109_get_tile_info2 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	k052109_get_tile_info(device, tileinfo, tile_index, 2, k052109->colorram_B, k052109->videoram_B, k052109->videoram2_B);
}


static STATE_POSTLOAD( k052109_tileflip_reset )
{
	k052109_state *k052109 = (k052109_state *)param;
	int data = k052109->ram[0x1e80];
	tilemap_set_flip(k052109->tilemap[0], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_flip(k052109->tilemap[1], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_flip(k052109->tilemap[2], (data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	k052109->tileflip_enable = ((data & 0x06) >> 1);
}

static DEVICE_START( k052109 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	const k052109_interface *intf = k052109_get_interface(device);
	running_machine *machine = device->machine;
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


	/* decode the graphics */
	switch (intf->plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, intf->gfx_memory_region) / 32;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout, 4);
		break;

	case GRADIUS3_PLANE_ORDER:
		total = 0x1000;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout_gradius3, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	/* deinterleave the graphics, if needed */
	deinterleave_gfx(machine, intf->gfx_memory_region, intf->deinterleave);

	k052109->memory_region = intf->gfx_memory_region;
	k052109->gfxnum = intf->gfx_num;
	k052109->callback = intf->callback;

	k052109->tilemap[0] = tilemap_create_device(device, k052109_get_tile_info0, tilemap_scan_rows, 8, 8, 64, 32);
	k052109->tilemap[1] = tilemap_create_device(device, k052109_get_tile_info1, tilemap_scan_rows, 8, 8, 64, 32);
	k052109->tilemap[2] = tilemap_create_device(device, k052109_get_tile_info2, tilemap_scan_rows, 8, 8, 64, 32);

	k052109->ram = auto_alloc_array_clear(machine, UINT8, 0x6000);

	k052109->colorram_F = &k052109->ram[0x0000];
	k052109->colorram_A = &k052109->ram[0x0800];
	k052109->colorram_B = &k052109->ram[0x1000];
	k052109->videoram_F = &k052109->ram[0x2000];
	k052109->videoram_A = &k052109->ram[0x2800];
	k052109->videoram_B = &k052109->ram[0x3000];
	k052109->videoram2_F = &k052109->ram[0x4000];
	k052109->videoram2_A = &k052109->ram[0x4800];
	k052109->videoram2_B = &k052109->ram[0x5000];

	tilemap_set_transparent_pen(k052109->tilemap[0], 0);
	tilemap_set_transparent_pen(k052109->tilemap[1], 0);
	tilemap_set_transparent_pen(k052109->tilemap[2], 0);

	state_save_register_device_item_pointer(device, 0, k052109->ram, 0x6000);
	state_save_register_device_item(device, 0, k052109->rmrd_line);
	state_save_register_device_item(device, 0, k052109->romsubbank);
	state_save_register_device_item(device, 0, k052109->scrollctrl);
	state_save_register_device_item(device, 0, k052109->irq_enabled);
	state_save_register_device_item_array(device, 0, k052109->charrombank);
	state_save_register_device_item_array(device, 0, k052109->charrombank_2);
	state_save_register_device_item_array(device, 0, k052109->dx);
	state_save_register_device_item_array(device, 0, k052109->dy);
	state_save_register_device_item(device, 0, k052109->has_extra_video_ram);
	state_save_register_postload(device->machine, k052109_tileflip_reset, k052109);
}

static DEVICE_RESET( k052109 )
{
	k052109_state *k052109 = k052109_get_safe_token(device);
	int i;

	k052109->rmrd_line = CLEAR_LINE;
	k052109->irq_enabled = 0;
	k052109->romsubbank = 0;
	k052109->scrollctrl = 0;

	k052109->has_extra_video_ram = 0;

	for (i = 0; i < 3; i++)
		k052109->dx[i] = k052109->dy[i] = 0;

	for (i = 0; i < 4; i++)
	{
		k052109->charrombank[i] = 0;
		k052109->charrombank_2[i] = 0;
	}
}

/***************************************************************************/
/*                                                                         */
/*                                 051960                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k051960_state k051960_state;
struct _k051960_state
{
	UINT8    *ram;

	gfx_element *gfx;

	UINT8    spriterombank[3];
	int      dx, dy;
	int      romoffset;
	int      spriteflip, readroms;
	int      irq_enabled, nmi_enabled;

	int      k051937_counter;

	k051960_callback callback;

	const char *memory_region;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k051960_state *k051960_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K051960);

	return (k051960_state *)device->token;
}

INLINE const k051960_interface *k051960_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K051960));
	return (const k051960_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static int k051960_fetchromdata( const device_config *device, int byte )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	int code, color, pri, shadow, off1, addr;

	addr = k051960->romoffset + (k051960->spriterombank[0] << 8) + ((k051960->spriterombank[1] & 0x03) << 16);
	code = (addr & 0x3ffe0) >> 5;
	off1 = addr & 0x1f;
	color = ((k051960->spriterombank[1] & 0xfc) >> 2) + ((k051960->spriterombank[2] & 0x03) << 6);
	pri = 0;
	shadow = color & 0x80;
	k051960->callback(device->machine, &code, &color, &pri, &shadow);

	addr = (code << 7) | (off1 << 2) | byte;
	addr &= memory_region_length(device->machine, k051960->memory_region) - 1;

//  popmessage("%s: addr %06x", cpuexec_describe_context(device->machine), addr);

	return memory_region(device->machine, k051960->memory_region)[addr];
}

READ8_DEVICE_HANDLER( k051960_r )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	if (k051960->readroms)
	{
		/* the 051960 remembers the last address read and uses it when reading the sprite ROMs */
		k051960->romoffset = (offset & 0x3fc) >> 2;
		return k051960_fetchromdata(device, offset & 3);	/* only 88 Games reads the ROMs from here */
	}
	else
		return k051960->ram[offset];
}

WRITE8_DEVICE_HANDLER( k051960_w )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	k051960->ram[offset] = data;
}

READ16_DEVICE_HANDLER( k051960_word_r )
{
	return k051960_r(device, offset * 2 + 1) | (k051960_r(device, offset * 2) << 8);
}

WRITE16_DEVICE_HANDLER( k051960_word_w )
{
	if (ACCESSING_BITS_8_15)
		k051960_w(device, offset * 2, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k051960_w(device, offset * 2 + 1, data & 0xff);
}


/* should this be split by k051960? */
READ8_DEVICE_HANDLER( k051937_r )
{
	k051960_state *k051960 = k051960_get_safe_token(device);

	if (k051960->readroms && offset >= 4 && offset < 8)
		return k051960_fetchromdata(device, offset & 3);
	else
	{
		if (offset == 0)
		{
			/* some games need bit 0 to pulse */
			return (k051960->k051937_counter++) & 1;
		}
		//logerror("%04x: read unknown 051937 address %x\n", cpu_get_pc(device->cpu), offset);
		return 0;
	}

	return 0;
}

WRITE8_DEVICE_HANDLER( k051937_w )
{
	k051960_state *k051960 = k051960_get_safe_token(device);

	if (offset == 0)
	{

		//if (data & 0xc2) popmessage("051937 reg 00 = %02x",data);

		/* bit 0 is IRQ enable */
		k051960->irq_enabled = data & 0x01;

		/* bit 1: probably FIRQ enable */

		/* bit 2 is NMI enable */
		k051960->nmi_enabled = data & 0x04;

		/* bit 3 = flip screen */
		k051960->spriteflip = data & 0x08;

		/* bit 4 used by Devastators and TMNT, unknown */

		/* bit 5 = enable gfx ROM reading */
		k051960->readroms = data & 0x20;
		//logerror("%04x: write %02x to 051937 address %x\n", cpu_get_pc(machine->cpu), data, offset);
	}
	else if (offset == 1)
	{
//  popmessage("%04x: write %02x to 051937 address %x", cpu_get_pc(machine->cpu), data, offset);
//logerror("%04x: write %02x to unknown 051937 address %x\n", cpu_get_pc(machine->cpu), data, offset);
	}
	else if (offset >= 2 && offset < 5)
	{
		k051960->spriterombank[offset - 2] = data;
	}
	else
	{
	//  popmessage("%04x: write %02x to 051937 address %x", cpu_get_pc(machine->cpu), data, offset);
	//logerror("%04x: write %02x to unknown 051937 address %x\n", cpu_get_pc(machine->cpu), data, offset);
	}
}

int k051960_is_irq_enabled( const device_config *device )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	return k051960->irq_enabled;
}

int k051960_is_nmi_enabled( const device_config *device )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	return k051960->nmi_enabled;
}

void k051960_set_sprite_offsets( const device_config *device, int dx, int dy )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	k051960->dx = dx;
	k051960->dy = dy;
}


READ16_DEVICE_HANDLER( k051937_word_r )
{
	return k051937_r(device, offset * 2 + 1) | (k051937_r(device, offset * 2) << 8);
}

WRITE16_DEVICE_HANDLER( k051937_word_w )
{
	if (ACCESSING_BITS_8_15)
		k051937_w(device, offset * 2,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k051937_w(device, offset * 2 + 1,data & 0xff);
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

void k051960_sprites_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int min_priority, int max_priority )
{
#define NUM_SPRITES 128
	k051960_state *k051960 = k051960_get_safe_token(device);
	running_machine *machine = device->machine;
	int offs, pri_code;
	int sortedlist[NUM_SPRITES];
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < 0x400; offs += 8)
	{
		if (k051960->ram[offs] & 0x80)
		{
			if (max_priority == -1)	/* draw front to back when using priority buffer */
				sortedlist[(k051960->ram[offs] & 0x7f) ^ 0x7f] = offs;
			else
				sortedlist[k051960->ram[offs] & 0x7f] = offs;
		}
	}

	for (pri_code = 0; pri_code < NUM_SPRITES; pri_code++)
	{
		int ox, oy, code, color, pri, shadow, size, w, h, x, y, flipx, flipy, zoomx, zoomy;
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
		if (offs == -1)
			continue;

		code = k051960->ram[offs + 2] + ((k051960->ram[offs + 1] & 0x1f) << 8);
		color = k051960->ram[offs + 3] & 0xff;
		pri = 0;
		shadow = color & 0x80;
		k051960->callback(device->machine, &code, &color, &pri, &shadow);

		if (max_priority != -1)
			if (pri < min_priority || pri > max_priority)
				continue;

		size = (k051960->ram[offs + 1] & 0xe0) >> 5;
		w = width[size];
		h = height[size];

		if (w >= 2) code &= ~0x01;
		if (h >= 2) code &= ~0x02;
		if (w >= 4) code &= ~0x04;
		if (h >= 4) code &= ~0x08;
		if (w >= 8) code &= ~0x10;
		if (h >= 8) code &= ~0x20;

		ox = (256 * k051960->ram[offs + 6] + k051960->ram[offs + 7]) & 0x01ff;
		oy = 256 - ((256 * k051960->ram[offs + 4] + k051960->ram[offs + 5]) & 0x01ff);
		ox += k051960->dx;
		oy += k051960->dy;
		flipx = k051960->ram[offs + 6] & 0x02;
		flipy = k051960->ram[offs + 4] & 0x02;
		zoomx = (k051960->ram[offs + 6] & 0xfc) >> 2;
		zoomy = (k051960->ram[offs + 4] & 0xfc) >> 2;
		zoomx = 0x10000 / 128 * (128 - zoomx);
		zoomy = 0x10000 / 128 * (128 - zoomy);

		if (k051960->spriteflip)
		{
			ox = 512 - (zoomx * w >> 12) - ox;
			oy = 256 - (zoomy * h >> 12) - oy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawmode_table[k051960->gfx->color_granularity - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		if (zoomx == 0x10000 && zoomy == 0x10000)
		{
			int sx, sy;

			for (y = 0; y < h; y++)
			{
				sy = oy + 16 * y;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + 16 * x;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (max_priority == -1)
						pdrawgfx_transtable(bitmap,cliprect,k051960->gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								machine->priority_bitmap,pri,
								drawmode_table,machine->shadow_table);
					else
						drawgfx_transtable(bitmap,cliprect,k051960->gfx,
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
			int sx, sy, zw, zh;

			for (y = 0; y < h; y++)
			{
				sy = oy + ((zoomy * y + (1 << 11)) >> 12);
				zh = (oy + ((zoomy * (y + 1) + (1 << 11)) >> 12)) - sy;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + ((zoomx * x + (1 << 11)) >> 12);
					zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (max_priority == -1)
						pdrawgfxzoom_transtable(bitmap,cliprect,k051960->gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								(zw << 16) / 16,(zh << 16) / 16,
								machine->priority_bitmap,pri,
								drawmode_table,machine->shadow_table);
					else
						drawgfxzoom_transtable(bitmap,cliprect,k051960->gfx,
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
		fwrite(k051960_ram, 0x400, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k051960 )
{
	k051960_state *k051960 = k051960_get_safe_token(device);
	const k051960_interface *intf = k051960_get_interface(device);
	running_machine *machine = device->machine;
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

	/* decode the graphics */
	switch (intf->plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, intf->gfx_memory_region) / 128;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &spritelayout, 4);
		break;

	case REVERSE_PLANE_ORDER:
		total = memory_region_length(machine, intf->gfx_memory_region) / 128;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &spritelayout_reverse, 4);
		break;

	case GRADIUS3_PLANE_ORDER:
		total = 0x4000;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &spritelayout_gradius3, 4);
		break;

	default:
		fatalerror("Unknown plane_order");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	/* deinterleave the graphics, if needed */
	deinterleave_gfx(machine, intf->gfx_memory_region, intf->deinterleave);

	k051960->memory_region = intf->gfx_memory_region;
	k051960->gfx = machine->gfx[intf->gfx_num];
	k051960->callback = intf->callback;
	k051960->ram = auto_alloc_array_clear(machine, UINT8, 0x400);

	state_save_register_device_item(device, 0, k051960->romoffset);
	state_save_register_device_item(device, 0, k051960->spriteflip);
	state_save_register_device_item(device, 0, k051960->readroms);
	state_save_register_device_item_array(device, 0, k051960->spriterombank);
	state_save_register_device_item_pointer(device, 0, k051960->ram, 0x400);
	state_save_register_device_item(device, 0, k051960->irq_enabled);
	state_save_register_device_item(device, 0, k051960->nmi_enabled);
	state_save_register_device_item(device, 0, k051960->dx);
	state_save_register_device_item(device, 0, k051960->dy);

	state_save_register_device_item(device, 0, k051960->k051937_counter);
}

static DEVICE_RESET( k051960 )
{
	k051960_state *k051960 = k051960_get_safe_token(device);

	k051960->dx = k051960->dy = 0;
	k051960->k051937_counter = 0;

	k051960->romoffset = 0;
	k051960->spriteflip = 0;
	k051960->readroms = 0;
	k051960->irq_enabled = 0;
	k051960->nmi_enabled = 0;

	k051960->spriterombank[0] = 0;
	k051960->spriterombank[1] = 0;
	k051960->spriterombank[2] = 0;
}

/***************************************************************************/
/*                                                                         */
/*                      05324x Family Sprite Generators                    */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*                         053244 / 053245                                 */
/*                                                                         */
/***************************************************************************/

typedef struct _k05324x_state k05324x_state;
struct _k05324x_state
{
	UINT16    *ram;
	UINT16    *buffer;

	gfx_element *gfx;

	UINT8    regs[0x10];	// 053244
	int      dx, dy;
	int      rombank;		// 053244
	int      ramsize;
	int      z_rejection;

	k05324x_callback callback;

	const char *memory_region;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k05324x_state *k05324x_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == K053244 || device->type == K053245));

	return (k05324x_state *)device->token;
}

INLINE const k05324x_interface *k05324x_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K053244 || device->type == K053245));
	return (const k05324x_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k053245_set_sprite_offs( const device_config *device, int offsx, int offsy )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	k053245->dx = offsx;
	k053245->dy = offsy;
}

READ16_DEVICE_HANDLER( k053245_word_r )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	return k053245->ram[offset];
}

WRITE16_DEVICE_HANDLER( k053245_word_w )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	COMBINE_DATA(k053245->ram + offset);
}

READ8_DEVICE_HANDLER( k053245_r )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);

	if(offset & 1)
		return k053245->ram[offset >> 1] & 0xff;
	else
		return (k053245->ram[offset >> 1] >> 8) & 0xff;
}


WRITE8_DEVICE_HANDLER( k053245_w )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);

	if(offset & 1)
		k053245->ram[offset >> 1] = (k053245->ram[offset >> 1] & 0xff00) | data;
	else
		k053245->ram[offset >> 1] = (k053245->ram[offset >> 1] & 0x00ff) | (data << 8);
}

void k053245_clear_buffer( const device_config *device )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	int i, e;

	for (e = k053245->ramsize / 2, i = 0; i < e; i += 8)
		k053245->buffer[i] = 0;
}

INLINE void k053245_update_buffer( const device_config *device )
{
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	memcpy(k053245->buffer, k053245->ram, k053245->ramsize);
}

READ8_DEVICE_HANDLER( k053244_r )
{
	k05324x_state *k053244 = k05324x_get_safe_token(device);
	running_machine *machine = device->machine;

	if ((k053244->regs[5] & 0x10) && offset >= 0x0c && offset < 0x10)
	{
		int addr;

		addr = (k053244->rombank << 19) | ((k053244->regs[11] & 0x7) << 18)
			| (k053244->regs[8] << 10) | (k053244->regs[9] << 2)
			| ((offset & 3) ^ 1);
		addr &= memory_region_length(machine, k053244->memory_region) - 1;

		//  popmessage("%s: offset %02x addr %06x", cpuexec_describe_context(machine), offset & 3, addr);

		return memory_region(machine, k053244->memory_region)[addr];
	}
	else if (offset == 0x06)
	{
		k053245_update_buffer(device);
		return 0;
	}
	else
	{
		//logerror("%s: read from unknown 053244 address %x\n", cpuexec_describe_context(machine), offset);
		return 0;
	}
}

WRITE8_DEVICE_HANDLER( k053244_w )
{
	k05324x_state *k053244 = k05324x_get_safe_token(device);

	k053244->regs[offset] = data;

	switch(offset)
	{
	case 0x05:
//      if (data & 0xc8)
//          popmessage("053244 reg 05 = %02x",data);
		/* bit 2 = unknown, Parodius uses it */
		/* bit 5 = unknown, Rollergames uses it */
//      logerror("%s: write %02x to 053244 address 5\n", cpuexec_describe_context(device->machine), data);
		break;

	case 0x06:
		k053245_update_buffer(device);
		break;
	}
}


READ16_DEVICE_HANDLER( k053244_lsb_r )
{
	return k053244_r(device, offset);
}

WRITE16_DEVICE_HANDLER( k053244_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		k053244_w(device, offset, data & 0xff);
}

READ16_DEVICE_HANDLER( k053244_word_r )
{
	return (k053244_r(device, offset * 2) << 8) | k053244_r(device, offset * 2 + 1);
}

WRITE16_DEVICE_HANDLER( k053244_word_w )
{
	if (ACCESSING_BITS_8_15)
		k053244_w(device, offset * 2, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k053244_w(device, offset * 2 + 1, data & 0xff);
}

void k053244_bankselect( const device_config *device, int bank )
{
	k05324x_state *k053244 = k05324x_get_safe_token(device);
	k053244->rombank = bank;
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

void k053245_sprites_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
#define NUM_SPRITES 128
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	running_machine *machine = device->machine;
	int offs, pri_code, i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = k053245->regs[5] & 0x01;
	flipscreenY = k053245->regs[5] & 0x02;
	spriteoffsX = (k053245->regs[0] << 8) | k053245->regs[1];
	spriteoffsY = (k053245->regs[2] << 8) | k053245->regs[3];

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i = k053245->ramsize / 2, offs = 0; offs < i; offs += 8)
	{
		pri_code = k053245->buffer[offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == k053245->z_rejection)
				continue;

			if (sortedlist[pri_code] == -1)
				sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		int ox, oy, color, code, size, w, h, x, y, flipx, flipy, mirrorx, mirrory, shadow, zoomx, zoomy, pri;

		offs = sortedlist[pri_code];
		if (offs == -1)
			continue;

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
		code = k053245->buffer[offs + 1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
				 + ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = k053245->buffer[offs + 6] & 0x00ff;
		pri = 0;

		k053245->callback(device->machine, &code, &color, &pri);

		size = (k053245->buffer[offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		zoomy = k053245->buffer[offs + 4];
		if (zoomy > 0x2000)
			continue;

		if (zoomy)
			zoomy = (0x400000 + zoomy / 2) / zoomy;
		else
			zoomy = 2 * 0x400000;
		if ((k053245->buffer[offs] & 0x4000) == 0)
		{
			zoomx = k053245->buffer[offs + 5];
			if (zoomx > 0x2000)
				continue;
			if (zoomx)
				zoomx = (0x400000 + zoomx / 2) / zoomx;
			else
				zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else
			zoomx = zoomy;

		ox = k053245->buffer[offs+3] + spriteoffsX;
		oy = k053245->buffer[offs+2];

		ox += k053245->dx;
		oy += k053245->dy;

		flipx = k053245->buffer[offs] & 0x1000;
		flipy = k053245->buffer[offs] & 0x2000;
		mirrorx = k053245->buffer[offs + 6] & 0x0100;
		if (mirrorx)
			flipx = 0; // documented and confirmed

		mirrory = k053245->buffer[offs + 6] & 0x0200;
		shadow = k053245->buffer[offs + 6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx)
				flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory)
				flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[k053245->gfx->color_granularity - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0; y < h; y++)
		{
			int sx, sy, zw, zh;

			sy = oy + ((zoomy * y + (1 << 11)) >> 12);
			zh = (oy + ((zoomy * (y + 1) + (1 << 11)) >> 12)) - sy;

			for (x = 0; x < w; x++)
			{
				int c, fx, fy;

				sx = ox + ((zoomx * x + (1 << 11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2*x < w))
					{
						/* mirror left/right */
						c += (w - x - 1);
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
						c += 8 * (h - y - 1);
						fy = 1;
					}
					else
					{
						c += 8 * y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8 * (h - 1 - y);
					else c += 8 * y;
					fy = flipy;
				}

				/* the sprite can start at any point in the 8x8 grid, but it must stay */
				/* in a 64 entries window, wrapping around at the edges. The animation */
				/* at the end of the saloon level in Sunset Riders breaks otherwise. */
				c = (c & 0x3f) | (code & ~0x3f);

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					pdrawgfx_transtable(bitmap,cliprect,k053245->gfx,
							c,
							color,
							fx,fy,
							sx,sy,
							machine->priority_bitmap,pri,
							drawmode_table,machine->shadow_table);
				}
				else
				{
					pdrawgfxzoom_transtable(bitmap,cliprect,k053245->gfx,
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
		fwrite(k053245->buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

/* Lethal Enforcers has 2 of these chips hooked up in parallel to give 6bpp gfx.. lets cheat a
  bit and make emulating it a little less messy by using a custom function instead */
void k053245_sprites_draw_lethal( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
#define NUM_SPRITES 128
	k05324x_state *k053245 = k05324x_get_safe_token(device);
	int offs, pri_code, i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];
	running_machine *machine = device->machine;

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = k053245->regs[5] & 0x01;
	flipscreenY = k053245->regs[5] & 0x02;
	spriteoffsX = (k053245->regs[0] << 8) | k053245->regs[1];
	spriteoffsY = (k053245->regs[2] << 8) | k053245->regs[3];

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i = k053245->ramsize / 2, offs = 0; offs < i; offs += 8)
	{
		pri_code = k053245->buffer[offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == k053245->z_rejection)
				continue;

			if (sortedlist[pri_code] == -1)
				sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		int ox, oy, color, code, size, w, h, x, y, flipx, flipy, mirrorx, mirrory, shadow, zoomx, zoomy, pri;

		offs = sortedlist[pri_code];
		if (offs == -1)
			continue;

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
		code = k053245->buffer[offs + 1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
				 + ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = k053245->buffer[offs + 6] & 0x00ff;
		pri = 0;

		k053245->callback(device->machine, &code, &color, &pri);

		size = (k053245->buffer[offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		zoomy = k053245->buffer[offs + 4];
		if (zoomy > 0x2000)
			continue;
		if (zoomy)
			zoomy = (0x400000 + zoomy / 2) / zoomy;
		else
			zoomy = 2 * 0x400000;
		if ((k053245->buffer[offs] & 0x4000) == 0)
		{
			zoomx = k053245->buffer[offs + 5];
			if (zoomx > 0x2000)
				continue;
			if (zoomx)
				zoomx = (0x400000 + zoomx / 2) / zoomx;
			else
				zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else
			zoomx = zoomy;

		ox = k053245->buffer[offs + 3] + spriteoffsX;
		oy = k053245->buffer[offs + 2];

		ox += k053245->dx;
		oy += k053245->dy;

		flipx = k053245->buffer[offs] & 0x1000;
		flipy = k053245->buffer[offs] & 0x2000;
		mirrorx = k053245->buffer[offs + 6] & 0x0100;
		if (mirrorx)
			flipx = 0; // documented and confirmed
		mirrory = k053245->buffer[offs + 6] & 0x0200;
		shadow = k053245->buffer[offs + 6] & 0x0080;

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

		drawmode_table[machine->gfx[0]->color_granularity - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0; y < h; y++)
		{
			int sx, sy, zw, zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0; x < w; x++)
			{
				int c, fx, fy;

				sx = ox + ((zoomx * x + (1 << 11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2 * x < w))
					{
						/* mirror left/right */
						c += (w - x - 1);
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
					if ((flipy == 0) ^ (2 * y >= h))
					{
						/* mirror top/bottom */
						c += 8 * (h - y - 1);
						fy = 1;
					}
					else
					{
						c += 8 * y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8 * (h - 1 - y);
					else c += 8 * y;
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
		fwrite(k053245->buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k05324x )
{
	k05324x_state *k05324x = k05324x_get_safe_token(device);
	const k05324x_interface *intf = k05324x_get_interface(device);
	running_machine *machine = device->machine;
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

	/* decode the graphics */
	switch (intf->plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, intf->gfx_memory_region) / 128;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	/* deinterleave the graphics, if needed */
	deinterleave_gfx(machine, intf->gfx_memory_region, intf->deinterleave);

	k05324x->ramsize = 0x800;

	k05324x->memory_region = intf->gfx_memory_region;
	k05324x->gfx = machine->gfx[intf->gfx_num];
	k05324x->dx = intf->dx;
	k05324x->dy = intf->dy;
	k05324x->callback = intf->callback;
	k05324x->ram = auto_alloc_array(machine, UINT16, k05324x->ramsize / 2);

	k05324x->buffer = auto_alloc_array(machine, UINT16, k05324x->ramsize / 2);

	state_save_register_device_item_pointer(device, 0, k05324x->ram, k05324x->ramsize / 2);
	state_save_register_device_item_pointer(device, 0, k05324x->buffer, k05324x->ramsize / 2);
	state_save_register_device_item(device, 0, k05324x->rombank);
	state_save_register_device_item(device, 0, k05324x->z_rejection);
	state_save_register_device_item_array(device, 0, k05324x->regs);
}

static DEVICE_RESET( k05324x )
{
	k05324x_state *k05324x = k05324x_get_safe_token(device);
	int i;

	k05324x->z_rejection = -1;
	k05324x->rombank = 0;

	for (i = 0; i < 0x10; i++)
		k05324x->regs[i] = 0;
}

/***************************************************************************/
/*                                                                         */
/*                                 053246/053247                           */
/*                                                                         */
/***************************************************************************/

typedef struct _k053247_state k053247_state;
struct _k053247_state
{
	UINT16    *ram;

	gfx_element *gfx;

	UINT8    kx46_regs[8];
	UINT16   kx47_regs[16];
	int      dx, dy, wraparound;
	UINT8    objcha_line;
	int      z_rejection;

	k05324x_callback callback;

	const char *memory_region;
	const device_config *screen;
};



/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053247_state *k053247_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == K053246 || device->type == K053247 || device->type == K055673));

	return (k053247_state *)device->token;
}

INLINE const k053247_interface *k053247_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K053246 || device->type == K053247 || device->type == K055673));
	return (const k053247_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

#if 0
void k053247_get_gfx( const device_config *device, gfx_element **gfx )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	*gfx = k053247->gfx;
}

void k053247_get_cb( const device_config *device, void (**callback)(int *, int *, int *) )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	*callback = k053247->callback;
}

#endif

void k053247_get_ram( const device_config *device, UINT16 **ram )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	*ram = k053247->ram;
}

int k053247_get_dx( const device_config *device )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return k053247->dx;
}

int k053247_get_dy( const device_config *device )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return k053247->dy;
}

int k053246_read_register( const device_config *device, int regnum )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return(k053247->kx46_regs[regnum]);
}

int k053247_read_register( const device_config *device, int regnum )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return(k053247->kx47_regs[regnum]);
}

void k053247_set_sprite_offs( const device_config *device, int offsx, int offsy )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	k053247->dx = offsx;
	k053247->dy = offsy;
}

void k053247_wraparound_enable( const device_config *device, int status )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	k053247->wraparound = status;
}

WRITE16_DEVICE_HANDLER( k053247_reg_word_w ) // write-only OBJSET2 registers (see p.43 table 6.1)
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	COMBINE_DATA(k053247->kx47_regs + offset);
}

WRITE32_DEVICE_HANDLER( k053247_reg_long_w )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	offset <<= 1;
	COMBINE_DATA(k053247->kx47_regs + offset + 1);
	mem_mask >>= 16;
	data >>= 16;
	COMBINE_DATA(k053247->kx47_regs + offset);
}

READ16_DEVICE_HANDLER( k053247_word_r )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return k053247->ram[offset];
}

WRITE16_DEVICE_HANDLER( k053247_word_w )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	COMBINE_DATA(k053247->ram + offset);
}

READ32_DEVICE_HANDLER( k053247_long_r )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return k053247->ram[offset * 2 + 1] | (k053247->ram[offset * 2] << 16);
}

WRITE32_DEVICE_HANDLER( k053247_long_w )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	offset <<= 1;
	COMBINE_DATA(k053247->ram + offset + 1);
	mem_mask >>= 16;
	data >>= 16;
	COMBINE_DATA(k053247->ram + offset);
}

READ8_DEVICE_HANDLER( k053247_r )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	int offs = offset >> 1;

	if (offset & 1)
		return(k053247->ram[offs] & 0xff);
	else
		return(k053247->ram[offs] >> 8);
}

WRITE8_DEVICE_HANDLER( k053247_w )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	int offs = offset >> 1;

	if (offset & 1)
		k053247->ram[offs] = (k053247->ram[offs] & 0xff00) | data;
	else
		k053247->ram[offs] = (k053247->ram[offs] & 0x00ff) | (data << 8);
}

// Mystic Warriors hardware games support a non-objcha based ROM readback
// write the address to the 246 as usual, but there's a completely separate ROM
// window that works without needing an objcha line.
// in this window, +0 = 32 bits from one set of ROMs, and +8 = 32 bits from another set
READ16_DEVICE_HANDLER( k055673_rom_word_r )	// 5bpp
{
	k053247_state *k053246 = k053247_get_safe_token(device);
	UINT8 *ROM8 = (UINT8 *)memory_region(device->machine, k053246->memory_region);
	UINT16 *ROM = (UINT16 *)memory_region(device->machine, k053246->memory_region);
	int size4 = (memory_region_length(device->machine, k053246->memory_region) / (1024 * 1024)) / 5;
	int romofs;

	size4 *= 4 * 1024 * 1024;	// get offset to 5th bit
	ROM8 += size4;

	romofs = k053246->kx46_regs[6] << 16 | k053246->kx46_regs[7] << 8 | k053246->kx46_regs[4];

	switch (offset)
	{
		case 0:	// 20k / 36u
			return ROM[romofs + 2];
		case 1:	// 17k / 36y
			return ROM[romofs + 3];
		case 2: // 10k / 32y
		case 3:
			romofs /= 2;
			return ROM8[romofs + 1];
		case 4:	// 22k / 34u
			return ROM[romofs];
		case 5:	// 19k / 34y
			return ROM[romofs + 1];
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

READ16_DEVICE_HANDLER( k055673_GX6bpp_rom_word_r )
{
	k053247_state *k053246 = k053247_get_safe_token(device);
	UINT16 *ROM = (UINT16 *)memory_region(device->machine, k053246->memory_region);
	int romofs;

	romofs = k053246->kx46_regs[6] << 16 | k053246->kx46_regs[7] << 8 | k053246->kx46_regs[4];

	romofs /= 4;	// romofs increments 4 at a time
	romofs *= 12 / 2;	// each increment of romofs = 12 new bytes (6 new words)

	switch (offset)
	{
		case 0:
			return ROM[romofs + 3];
		case 1:
			return ROM[romofs + 4];
		case 2:
		case 3:
			return ROM[romofs + 5];
		case 4:
			return ROM[romofs];
		case 5:
			return ROM[romofs + 1];
		case 6:
		case 7:
			return ROM[romofs + 2];
		default:
//          LOG(("55673_rom_word_r: Unknown read offset %x (PC=%x)\n", offset, cpu_get_pc(space->cpu)));
			break;
	}

	return 0;
}

READ8_DEVICE_HANDLER( k053246_r )
{
	k053247_state *k053246 = k053247_get_safe_token(device);
	if (k053246->objcha_line == ASSERT_LINE)
	{
		int addr;

		addr = (k053246->kx46_regs[6] << 17) | (k053246->kx46_regs[7] << 9) | (k053246->kx46_regs[4] << 1) | ((offset & 1) ^ 1);
		addr &= memory_region_length(device->machine, k053246->memory_region) - 1;
//      if (VERBOSE)
//          popmessage("%04x: offset %02x addr %06x", cpu_get_pc(space->cpu), offset, addr);
		return memory_region(device->machine, k053246->memory_region)[addr];
	}
	else
	{
//      LOG(("%04x: read from unknown 053246 address %x\n", cpu_get_pc(space->cpu), offset));
		return 0;
	}
}

WRITE8_DEVICE_HANDLER( k053246_w )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	k053247->kx46_regs[offset] = data;
}

READ16_DEVICE_HANDLER( k053246_word_r )
{
	offset <<= 1;
	return k053246_r(device, offset + 1) | (k053246_r(device, offset) << 8);
}

WRITE16_DEVICE_HANDLER( k053246_word_w )
{
	if (ACCESSING_BITS_8_15)
		k053246_w(device, offset << 1,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		k053246_w(device, (offset << 1) + 1,data & 0xff);
}

READ32_DEVICE_HANDLER( k053246_long_r )
{
	offset <<= 1;
	return (k053246_word_r(device, offset + 1, 0xffff) | k053246_word_r(device, offset, 0xffff) << 16);
}

WRITE32_DEVICE_HANDLER( k053246_long_w )
{
	offset <<= 1;
	k053246_word_w(device, offset, data >> 16, mem_mask >> 16);
	k053246_word_w(device, offset + 1, data, mem_mask);
}

void k053246_set_objcha_line( const device_config *device, int state )
{
	k053247_state *k053246 = k053247_get_safe_token(device);
	k053246->objcha_line = state;
}

int k053246_is_irq_enabled( const device_config *device )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	// This bit enables obj DMA rather than obj IRQ even though the two functions usually coincide.
	return k053247->kx46_regs[5] & 0x10;
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

void k053247_sprites_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
#define NUM_SPRITES 256
	k053247_state *k053246 = k053247_get_safe_token(device);
	running_machine *machine = device->machine;

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
	int ox, oy, color, code, size, w, h, x, y, xa, ya, flipx, flipy, mirrorx, mirrory, shadow, zoomx, zoomy, primask;
	int shdmask, nozoom, count, temp;

	int flipscreenx = k053246->kx46_regs[5] & 0x01;
	int flipscreeny = k053246->kx46_regs[5] & 0x02;
	int offx = (short)((k053246->kx46_regs[0] << 8) | k053246->kx46_regs[1]);
	int offy = (short)((k053246->kx46_regs[2] << 8) | k053246->kx46_regs[3]);

	int screen_width = video_screen_get_width(k053246->screen);
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
        The k053247 does not draw pixels on top of those with equal or smaller Z-values
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

        These are a small part of the k053247's feature set but many games expect them
        to be the minimum compliances. The specification will undoubtedly require
        redesigning the priority system from the ground up. Drawgfx.c and tilemap.c must
        also undergo heavy facelifts but in the end the changes could hurt simpler games
        more than they help complex systems; therefore the new engine should remain
        completely stand alone and self-contained. Implementation details are being
        hammered down but too early to make propositions.
    */

	// Prebuild a sorted table by descending Z-order.
	zcode = k053246->z_rejection;
	offs = count = 0;

	if (zcode == -1)
	{
		for (; offs < 0x800; offs += 8)
			if (k053246->ram[offs] & 0x8000)
				sortedlist[count++] = offs;
	}
	else
	{
		for (; offs < 0x800; offs += 8)
			if ((k053246->ram[offs] & 0x8000) && ((k053246->ram[offs] & 0xff) != zcode))
				sortedlist[count++] = offs;
	}

	w = count;
	count--;
	h = count;

	if (!(k053246->kx47_regs[0xc / 2] & 0x10))
	{
		// sort objects in decending order(smaller z closer) when OPSET PRI is clear
		for (y = 0; y < h; y++)
		{
			offs = sortedlist[y];
			zcode = k053246->ram[offs] & 0xff;
			for (x = y + 1; x < w; x++)
			{
				temp = sortedlist[x];
				code = k053246->ram[temp] & 0xff;
				if (zcode <= code)
				{
					zcode = code;
					sortedlist[x] = offs;
					sortedlist[y] = offs = temp;
				}
			}
		}
	}
	else
	{
		// sort objects in ascending order(bigger z closer) when OPSET PRI is set
		for (y = 0; y < h; y++)
		{
			offs = sortedlist[y];
			zcode = k053246->ram[offs] & 0xff;
			for (x = y + 1; x < w; x++)
			{
				temp = sortedlist[x];
				code = k053246->ram[temp] & 0xff;
				if (zcode >= code)
				{
					zcode = code;
					sortedlist[x] = offs;
					sortedlist[y] = offs = temp;
				}
			}
		}
	}

	for (; count >= 0; count--)
	{
		offs = sortedlist[count];

		code = k053246->ram[offs + 1];
		shadow = color = k053246->ram[offs + 6];
		primask = 0;

		k053246->callback(device->machine, &code, &color, &primask);

		temp = k053246->ram[offs];

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

		oy = (short)k053246->ram[offs + 2];
		ox = (short)k053246->ram[offs + 3];

		if (k053246->wraparound)
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
		y = zoomy = k053246->ram[offs + 4] & 0x3ff;
		if (zoomy)
			zoomy = (0x400000 + (zoomy >> 1)) / zoomy;
		else
			zoomy = 0x800000;

		if (!(temp & 0x4000))
		{
			x = zoomx = k053246->ram[offs + 5] & 0x3ff;
			if (zoomx)
				zoomx = (0x400000 + (zoomx >> 1)) / zoomx;
			else
				zoomx = 0x800000;
		}
		else
		{
			zoomx = zoomy;
			x = y;
		}

// ************************************************************************************
//  for Escape Kids (GX975)
// ************************************************************************************
//    Escape Kids use 053246 #5 register's UNKNOWN Bit #5, #3 and #2.
//    Bit #5, #3, #2 always set "1".
//    Maybe, Bit #5 or #3 or #2 or combination means "FIX SPRITE WIDTH TO HALF" ?????
//    Below 7 lines supports this 053246's(???) function.
//    Don't rely on it, Please.  But, Escape Kids works correctly!
// ************************************************************************************
		if ( k053246->kx46_regs[5] & 0x08 ) // Check only "Bit #3 is '1'?" (NOTE: good guess)
		{
			zoomx >>= 1;		// Fix sprite width to HALF size
			ox = (ox >> 1) + 1;	// Fix sprite draw position
			if (flipscreenx)
				ox += screen_width;
			nozoom = 0;
		}
		else
			nozoom = (x == 0x40 && y == 0x40);

		flipx = temp & 0x1000;
		flipy = temp & 0x2000;
		mirrorx = shadow & 0x4000;
		if (mirrorx)
			flipx = 0; // documented and confirmed
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
				shadow = (color & K053247_CUSTOMSHADOW) ? (color >> K053247_SHDSHIFT) : (shadow >> 10);
				if (shadow &= 3) palette_set_shadow_mode(machine, (shadow - 1) & shdmask);
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
		if (k053246->wraparound)
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
		ox += k053246->dx;
		oy -= k053246->dy;

		// apply global and display window offsets

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[k053246->gfx->color_granularity - 1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0; y < h; y++)
		{
			int sx, sy, zw, zh;

			sy = oy + ((zoomy * y + (1 << 11)) >> 12);
			zh = (oy + ((zoomy * (y + 1) + (1 << 11)) >> 12)) - sy;

			for (x = 0; x < w; x++)
			{
				int c, fx, fy;

				sx = ox + ((zoomx * x + (1 << 11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1 << 11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ ((x << 1) < w))
					{
						/* mirror left/right */
						c += xoffset[(w - 1 - x + xa) & 7];
						fx = 1;
					}
					else
					{
						c += xoffset[(x + xa) & 7];
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += xoffset[(w - 1 - x + xa) & 7];
					else c += xoffset[(x + xa) & 7];
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ ((y<<1) >= h))
					{
						/* mirror top/bottom */
						c += yoffset[(h - 1 - y + ya) & 7];
						fy = 1;
					}
					else
					{
						c += yoffset[(y + ya) & 7];
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += yoffset[(h - 1 - y + ya) & 7];
					else c += yoffset[(y + ya) & 7];
					fy = flipy;
				}

				if (nozoom)
				{
					pdrawgfx_transtable(bitmap,cliprect,k053246->gfx,
							c,
							color,
							fx,fy,
							sx,sy,
							machine->priority_bitmap,primask,
							whichtable,machine->shadow_table);
				}
				else
				{
					pdrawgfxzoom_transtable(bitmap,cliprect,k053246->gfx,
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
						pdrawgfx_transtable(bitmap,cliprect,k053246->gfx,
								c,
								color,
								fx,!fy,
								sx,sy,
								machine->priority_bitmap,primask,
								whichtable,machine->shadow_table);
					}
					else
					{
						pdrawgfxzoom_transtable(bitmap,cliprect,k053246->gfx,
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


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053247 )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	const k053247_interface *intf = k053247_get_interface(device);
	running_machine *machine = device->machine;
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

	k053247->screen = devtag_get_device(device->machine, intf->screen);

	/* decode the graphics */
	switch (intf->plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, intf->gfx_memory_region) / 128;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	if (VERBOSE)
	{
		if (video_screen_get_format(k053247->screen) == BITMAP_FORMAT_RGB32)
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

	/* deinterleave the graphics, if needed */
	deinterleave_gfx(machine, intf->gfx_memory_region, intf->deinterleave);

	k053247->dx = intf->dx;
	k053247->dy = intf->dy;
	k053247->memory_region = intf->gfx_memory_region;
	k053247->gfx = machine->gfx[intf->gfx_num];
	k053247->callback = intf->callback;

	k053247->ram = auto_alloc_array_clear(machine, UINT16, 0x1000 / 2);

	state_save_register_device_item_pointer(device, 0, k053247->ram, 0x1000 / 2);
	state_save_register_device_item_array(device, 0, k053247->kx46_regs);
	state_save_register_device_item_array(device, 0, k053247->kx47_regs);
	state_save_register_device_item(device, 0, k053247->objcha_line);
	state_save_register_device_item(device, 0, k053247->wraparound);
	state_save_register_device_item(device, 0, k053247->z_rejection);
}

/* K055673 used with the 54246 in PreGX/Run and Gun/System GX games */
static DEVICE_START( k055673 )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	const k053247_interface *intf = k053247_get_interface(device);
	running_machine *machine = device->machine;
	UINT32 total;
	UINT8 *s1, *s2, *d;
	long i;
	UINT16 *K055673_rom;
	int size4;

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

	k053247->screen = devtag_get_device(device->machine, intf->screen);

	K055673_rom = (UINT16 *)memory_region(machine, intf->gfx_memory_region);

	/* decode the graphics */
	switch (intf->plane_order)	/* layout would be more correct than plane_order, but we use k053247_interface */
	{
	case K055673_LAYOUT_GX:
		size4 = (memory_region_length(machine, intf->gfx_memory_region) / (1024 * 1024)) / 5;
		size4 *= 4 * 1024 * 1024;
		/* set the # of tiles based on the 4bpp section */
		K055673_rom = auto_alloc_array(machine, UINT16, size4 * 5 / 2);
		d = (UINT8 *)K055673_rom;
		// now combine the graphics together to form 5bpp
		s1 = memory_region(machine, intf->gfx_memory_region); // 4bpp area
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
		decode_gfx(machine, intf->gfx_num, (UINT8 *)K055673_rom, total, &spritelayout, 4);
		break;

	case K055673_LAYOUT_RNG:
		total = memory_region_length(machine, intf->gfx_memory_region) / (16 * 16 / 2);
		decode_gfx(machine, intf->gfx_num, (UINT8 *)K055673_rom, total, &spritelayout2, 4);
		break;

	case K055673_LAYOUT_LE2:
		total = memory_region_length(machine, intf->gfx_memory_region) / (16 * 16);
		decode_gfx(machine, intf->gfx_num, (UINT8 *)K055673_rom, total, &spritelayout3, 4);
		break;

	case K055673_LAYOUT_GX6:
		total = memory_region_length(machine, intf->gfx_memory_region) / (16 * 16 * 6 / 8);
		decode_gfx(machine, intf->gfx_num, (UINT8 *)K055673_rom, total, &spritelayout4, 4);
		break;

	default:
		fatalerror("Unsupported layout");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	k053247->dx = intf->dx;
	k053247->dy = intf->dy;
	k053247->memory_region = intf->gfx_memory_region;
	k053247->gfx = machine->gfx[intf->gfx_num];
	k053247->callback = intf->callback;

	k053247->ram = auto_alloc_array(machine, UINT16, 0x1000 / 2);

	state_save_register_device_item_pointer(device, 0, k053247->ram, 0x800);
	state_save_register_device_item_array(device, 0, k053247->kx46_regs);
	state_save_register_device_item_array(device, 0, k053247->kx47_regs);
	state_save_register_device_item(device, 0, k053247->objcha_line);
	state_save_register_device_item(device, 0, k053247->wraparound);
	state_save_register_device_item(device, 0, k053247->z_rejection);
}

static DEVICE_RESET( k053247 )
{
	k053247_state *k053247 = k053247_get_safe_token(device);

	k053247->wraparound = 1;
	k053247->z_rejection = -1;
	k053247->objcha_line = CLEAR_LINE;

	memset(k053247->kx46_regs, 0, 8);
	memset(k053247->kx47_regs, 0, 32);
}

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


void k05324x_set_z_rejection( const device_config *device, int zcode )
{
	k05324x_state *k05324x = k05324x_get_safe_token(device);
	k05324x->z_rejection = zcode;
}

void k053247_set_z_rejection( const device_config *device, int zcode )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	k053247->z_rejection = zcode;
}

/***************************************************************************/
/*                                                                         */
/*                                 051316                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k051316_state k051316_state;
struct _k051316_state
{
	UINT8    *ram;

	tilemap_t  *tmap;

	int      gfxnum, wraparound, bpp;
	int      offset[2];
	UINT8    ctrlram[16];

	k051316_callback callback;

	const char *memory_region;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k051316_state *k051316_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K051316);

	return (k051316_state *)device->token;
}

INLINE const k051316_interface *k051316_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert(device->type == K051316);
	return (const k051316_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k051316_r )
{
	k051316_state *k051316= k051316_get_safe_token(device);
	return k051316->ram[offset];
}

WRITE8_DEVICE_HANDLER( k051316_w )
{
	k051316_state *k051316= k051316_get_safe_token(device);

	k051316->ram[offset] = data;
	tilemap_mark_tile_dirty(k051316->tmap, offset & 0x3ff);
}


READ8_DEVICE_HANDLER( k051316_rom_r )
{
	k051316_state *k051316= k051316_get_safe_token(device);

	if ((k051316->ctrlram[0x0e] & 0x01) == 0)
	{
		int addr = offset + (k051316->ctrlram[0x0c] << 11) + (k051316->ctrlram[0x0d] << 19);
		if (k051316->bpp <= 4)
			addr /= 2;
		addr &= memory_region_length(device->machine, k051316->memory_region) - 1;

		//  popmessage("%s: offset %04x addr %04x", cpuexec_describe_context(device->machine), offset, addr);

		return memory_region(device->machine, k051316->memory_region)[addr];
	}
	else
	{
		//logerror("%s: read 051316 ROM offset %04x but reg 0x0c bit 0 not clear\n", cpuexec_describe_context(device->machine), offset);
		return 0;
	}
}

WRITE8_DEVICE_HANDLER( k051316_ctrl_w )
{
	k051316_state *k051316= k051316_get_safe_token(device);
	k051316->ctrlram[offset] = data;
	//if (offset >= 0x0c) logerror("%s: write %02x to 051316 reg %x\n", cpuexec_describe_context(device->machine), data, offset);
}

// a few games (ajax, rollerg, ultraman, etc.) can enable and disable wraparound after start
void k051316_wraparound_enable( const device_config *device, int status )
{
	k051316_state *k051316= k051316_get_safe_token(device);
	k051316->wraparound = status;
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void k051316_get_tile_info( const device_config *device, tile_data *tileinfo, int tile_index )
{
	k051316_state *k051316 = k051316_get_safe_token(device);
	int code = k051316->ram[tile_index];
	int color = k051316->ram[tile_index + 0x400];
	int flags = 0;

	k051316->callback(device->machine, &code, &color, &flags);

	SET_TILE_INFO_DEVICE(
			k051316->gfxnum,
			code,
			color,
			flags);
}


static TILE_GET_INFO_DEVICE( k051316_get_tile_info0 ) { k051316_get_tile_info(device, tileinfo, tile_index); }


void k051316_zoom_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int flags, UINT32 priority )
{
	k051316_state *k051316= k051316_get_safe_token(device);
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	startx = 256 * ((INT16)(256 * k051316->ctrlram[0x00] + k051316->ctrlram[0x01]));
	incxx  =        (INT16)(256 * k051316->ctrlram[0x02] + k051316->ctrlram[0x03]);
	incyx  =        (INT16)(256 * k051316->ctrlram[0x04] + k051316->ctrlram[0x05]);
	starty = 256 * ((INT16)(256 * k051316->ctrlram[0x06] + k051316->ctrlram[0x07]));
	incxy  =        (INT16)(256 * k051316->ctrlram[0x08] + k051316->ctrlram[0x09]);
	incyy  =        (INT16)(256 * k051316->ctrlram[0x0a] + k051316->ctrlram[0x0b]);

	startx -= (16 + k051316->offset[1]) * incyx;
	starty -= (16 + k051316->offset[1]) * incyy;

	startx -= (89 + k051316->offset[0]) * incxx;
	starty -= (89 + k051316->offset[0]) * incxy;

	tilemap_draw_roz(bitmap,cliprect,k051316->tmap,startx << 5,starty << 5,
			incxx << 5,incxy << 5,incyx << 5,incyy << 5,
			k051316->wraparound,
			flags,priority);

#if 0
	popmessage("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
			k051316->ctrlram[0x00],
			k051316->ctrlram[0x01],
			k051316->ctrlram[0x02],
			k051316->ctrlram[0x03],
			k051316->ctrlram[0x04],
			k051316->ctrlram[0x05],
			k051316->ctrlram[0x06],
			k051316->ctrlram[0x07],
			k051316->ctrlram[0x08],
			k051316->ctrlram[0x09],
			k051316->ctrlram[0x0a],
			k051316->ctrlram[0x0b],
			k051316->ctrlram[0x0c],	/* bank for ROM testing */
			k051316->ctrlram[0x0d],
			k051316->ctrlram[0x0e],	/* 0 = test ROMs */
			k051316->ctrlram[0x0f]);
#endif
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k051316 )
{
	k051316_state *k051316 = k051316_get_safe_token(device);
	const k051316_interface *intf = k051316_get_interface(device);
	running_machine *machine = device->machine;

	int is_tail2nos = 0;
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

	/* decode the graphics */
	switch (intf->bpp)
	{
	case -4:
		total = 0x400;
		is_tail2nos = 1;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout_tail2nos, 4);
		break;

	case 4:
		total = memory_region_length(machine, intf->gfx_memory_region) / 128;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout4, 4);
		break;

	case 7:
		total = memory_region_length(machine, intf->gfx_memory_region) / 256;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout7, 7);
		break;

	case 8:
		total = memory_region_length(machine, intf->gfx_memory_region) / 256;
		decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout8, 8);
		break;

	default:
		fatalerror("Unsupported bpp");
	}

	k051316->memory_region = intf->gfx_memory_region;
	k051316->gfxnum = intf->gfx_num;
	k051316->bpp = is_tail2nos ? 4 : intf->bpp;	// tail2nos is passed with bpp = -4 to setup the custom charlayout!
	k051316->callback = intf->callback;

	k051316->tmap = tilemap_create_device(device, k051316_get_tile_info0, tilemap_scan_rows, 16, 16, 32, 32);

	k051316->ram = auto_alloc_array(machine, UINT8, 0x800);

	if (!intf->pen_is_mask)
		tilemap_set_transparent_pen(k051316->tmap, intf->transparent_pen);
	else
	{
		tilemap_map_pens_to_layer(k051316->tmap, 0, 0, 0, TILEMAP_PIXEL_LAYER1);
		tilemap_map_pens_to_layer(k051316->tmap, 0, intf->transparent_pen, intf->transparent_pen, TILEMAP_PIXEL_LAYER0);
	}

	k051316->wraparound = intf->wrap;
	k051316->offset[0] = intf->xoffs;
	k051316->offset[1] = intf->yoffs;

	state_save_register_device_item_pointer(device, 0, k051316->ram, 0x800);
	state_save_register_device_item_array(device, 0, k051316->ctrlram);
	state_save_register_device_item(device, 0, k051316->wraparound);
}

static DEVICE_RESET( k051316 )
{
	k051316_state *k051316 = k051316_get_safe_token(device);

	memset(k051316->ctrlram,  0, 0x10);
}

/***************************************************************************/
/*                                                                         */
/*                                 053936                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k053936_state k053936_state;
struct _k053936_state
{
	UINT16    *ctrl;
	UINT16    *linectrl;

	int      wraparound;
	int      offset[2];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053936_state *k053936_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K053936);

	return (k053936_state *)device->token;
}

INLINE const k053936_interface *k053936_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert(device->type == K053936);
	return (const k053936_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE16_DEVICE_HANDLER( k053936_ctrl_w )
{
	k053936_state *k053936= k053936_get_safe_token(device);
	COMBINE_DATA(&k053936->ctrl[offset]);
}

/* FIXME: this is probably unused... check! */
READ16_DEVICE_HANDLER( k053936_ctrl_r )
{
	k053936_state *k053936= k053936_get_safe_token(device);
	return k053936->ctrl[offset];
}

WRITE16_DEVICE_HANDLER( k053936_linectrl_w )
{
	k053936_state *k053936= k053936_get_safe_token(device);
	COMBINE_DATA(&k053936->linectrl[offset]);
}

READ16_DEVICE_HANDLER( k053936_linectrl_r )
{
	k053936_state *k053936= k053936_get_safe_token(device);
	return k053936->linectrl[offset];
}

// there is another implementation of this in  video/konamigx.c (!)
//  why? shall they be merged?
void k053936_zoom_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, tilemap_t *tmap, int flags, UINT32 priority, int glfgreat_hack )
{
	k053936_state *k053936= k053936_get_safe_token(device);
	if (!tmap)
		return;

	if (k053936->ctrl[0x07] & 0x0040)
	{
		UINT32 startx, starty;
		int incxx, incxy;
		rectangle my_clip;
		int y, maxy;

		// Racin' Force will get to here if glfgreat_hack is enabled, and it ends
		// up setting a maximum y value of '13', thus causing nothing to be drawn.
		// It looks like the roz output should be flipped somehow as it seems to be
		// displaying the wrong areas of the tilemap and is rendered upside down,
		// although due to the additional post-processing the voxel renderer performs
		// it's difficult to know what the output SHOULD be.  (hold W in Racin' Force
		// to see the chip output)

		if (((k053936->ctrl[0x07] & 0x0002) && k053936->ctrl[0x09]) && (glfgreat_hack))	/* wrong, but fixes glfgreat */
		{
			my_clip.min_x = k053936->ctrl[0x08] + k053936->offset[0] + 2;
			my_clip.max_x = k053936->ctrl[0x09] + k053936->offset[0] + 2 - 1;
			if (my_clip.min_x < cliprect->min_x)
				my_clip.min_x = cliprect->min_x;
			if (my_clip.max_x > cliprect->max_x)
				my_clip.max_x = cliprect->max_x;

			y = k053936->ctrl[0x0a] + k053936->offset[1] - 2;
			if (y < cliprect->min_y)
				y = cliprect->min_y;
			maxy = k053936->ctrl[0x0b] + k053936->offset[1] - 2 - 1;
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
			UINT16 *lineaddr = k053936->linectrl + 4 * ((y - k053936->offset[1]) & 0x1ff);

			my_clip.min_y = my_clip.max_y = y;

			startx = 256 * (INT16)(lineaddr[0] + k053936->ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + k053936->ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (k053936->ctrl[0x06] & 0x8000)
				incxx *= 256;

			if (k053936->ctrl[0x06] & 0x0080)
				incxy *= 256;

			startx -= k053936->offset[0] * incxx;
			starty -= k053936->offset[0] * incxy;

			tilemap_draw_roz(bitmap,&my_clip,tmap,startx << 5,starty << 5,
					incxx << 5,incxy << 5,0,0,
					k053936->wraparound,
					flags,priority);

			y++;
		}
	}
	else	/* "simple" mode */
	{
		UINT32 startx, starty;
		int incxx, incxy, incyx, incyy;

		startx = 256 * (INT16)(k053936->ctrl[0x00]);
		starty = 256 * (INT16)(k053936->ctrl[0x01]);
		incyx  =       (INT16)(k053936->ctrl[0x02]);
		incyy  =       (INT16)(k053936->ctrl[0x03]);
		incxx  =       (INT16)(k053936->ctrl[0x04]);
		incxy  =       (INT16)(k053936->ctrl[0x05]);

		if (k053936->ctrl[0x06] & 0x4000)
		{
			incyx *= 256;
			incyy *= 256;
		}

		if (k053936->ctrl[0x06] & 0x0040)
		{
			incxx *= 256;
			incxy *= 256;
		}

		startx -= k053936->offset[1] * incyx;
		starty -= k053936->offset[1] * incyy;

		startx -= k053936->offset[0] * incxx;
		starty -= k053936->offset[0] * incxy;

		tilemap_draw_roz(bitmap,cliprect,tmap,startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				k053936->wraparound,
				flags,priority);
	}

#if 0
if (input_code_pressed(machine, KEYCODE_D))
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x",
			k053936->ctrl[0x00],
			k053936->ctrl[0x01],
			k053936->ctrl[0x02],
			k053936->ctrl[0x03],
			k053936->ctrl[0x04],
			k053936->ctrl[0x05],
			k053936->ctrl[0x06],
			k053936->ctrl[0x07],
			k053936->ctrl[0x08],
			k053936->ctrl[0x09],
			k053936->ctrl[0x0a],
			k053936->ctrl[0x0b],
			k053936->ctrl[0x0c],
			k053936->ctrl[0x0d],
			k053936->ctrl[0x0e],
			k053936->ctrl[0x0f]);
#endif
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053936 )
{
	k053936_state *k053936 = k053936_get_safe_token(device);
	const k053936_interface *intf = k053936_get_interface(device);

	k053936->ctrl = auto_alloc_array(device->machine, UINT16, 0x20);
	k053936->linectrl = auto_alloc_array(device->machine, UINT16, 0x4000);

	k053936->wraparound = intf->wrap;
	k053936->offset[0] = intf->xoff;
	k053936->offset[1] = intf->yoff;

	state_save_register_device_item_pointer(device, 0, k053936->ctrl, 0x20);
	state_save_register_device_item_pointer(device, 0, k053936->linectrl, 0x4000);
}

static DEVICE_RESET( k053936 )
{
	k053936_state *k053936 = k053936_get_safe_token(device);

	memset(k053936->ctrl, 0, 0x20);
}

/***************************************************************************/
/*                                                                         */
/*                                 053251                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k053251_state k053251_state;
struct _k053251_state
{
	int      dirty_tmap[5];

	UINT8    ram[16];
	int      tilemaps_set;
	int      palette_index[5];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053251_state *k053251_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K053251);

	return (k053251_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( k053251_w )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	int i, newind;

	data &= 0x3f;

	if (k053251->ram[offset] != data)
	{
		k053251->ram[offset] = data;
		if (offset == 9)
		{
			/* palette base index */
			for (i = 0; i < 3; i++)
			{
				newind = 32 * ((data >> 2 * i) & 0x03);
				if (k053251->palette_index[i] != newind)
				{
					k053251->palette_index[i] = newind;
					k053251->dirty_tmap[i] = 1;
				}
			}

			if (!k053251->tilemaps_set)
				tilemap_mark_all_tiles_dirty_all(device->machine);
		}
		else if (offset == 10)
		{
			/* palette base index */
			for (i = 0; i < 2; i++)
			{
				newind = 16 * ((data >> 3 * i) & 0x07);
				if (k053251->palette_index[3 + i] != newind)
				{
					k053251->palette_index[3 + i] = newind;
					k053251->dirty_tmap[3 + i] = 1;
				}
			}

			if (!k053251->tilemaps_set)
				tilemap_mark_all_tiles_dirty_all(device->machine);
		}
	}
}

WRITE16_DEVICE_HANDLER( k053251_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		k053251_w(device, offset, data & 0xff);
}

WRITE16_DEVICE_HANDLER( k053251_msb_w )
{
	if (ACCESSING_BITS_8_15)
		k053251_w(device, offset, (data >> 8) & 0xff);
}

int k053251_get_priority( const device_config *device, int ci )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	return k053251->ram[ci];
}

int k053251_get_palette_index( const device_config *device, int ci )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	return k053251->palette_index[ci];
}

int k053251_get_tmap_dirty( const device_config *device, int tmap_num )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	assert(tmap_num < 5);
	return k053251->dirty_tmap[tmap_num];
}

void k053251_set_tmap_dirty( const device_config *device, int tmap_num, int data )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	assert(tmap_num < 5);
	k053251->dirty_tmap[tmap_num] = data ? 1 : 0;
}

static STATE_POSTLOAD( k053251_reset_indexes )
{
	k053251_state *k053251 = (k053251_state *)param;

	k053251->palette_index[0] = 32 * ((k053251->ram[9] >> 0) & 0x03);
	k053251->palette_index[1] = 32 * ((k053251->ram[9] >> 2) & 0x03);
	k053251->palette_index[2] = 32 * ((k053251->ram[9] >> 4) & 0x03);
	k053251->palette_index[3] = 16 * ((k053251->ram[10] >> 0) & 0x07);
	k053251->palette_index[4] = 16 * ((k053251->ram[10] >> 3) & 0x07);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053251 )
{
	k053251_state *k053251 = k053251_get_safe_token(device);

	state_save_register_device_item_array(device, 0, k053251->ram);
	state_save_register_device_item(device, 0, k053251->tilemaps_set);
	state_save_register_device_item_array(device, 0, k053251->dirty_tmap);

	state_save_register_postload(device->machine, k053251_reset_indexes, k053251);
}

static DEVICE_RESET( k053251 )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	int i;

	k053251->tilemaps_set = 0;

	for (i = 0; i < 0x10; i++)
		k053251->ram[i] = 0;

	for (i = 0; i < 5; i++)
		k053251->dirty_tmap[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k054000_state k054000_state;
struct _k054000_state
{
	UINT8    regs[0x20];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k054000_state *k054000_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K054000);

	return (k054000_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( k054000_w )
{
	k054000_state *k054000 = k054000_get_safe_token(device);

	//logerror("%04x: write %02x to 054000 address %02x\n",cpu_get_pc(space->cpu),data,offset);
	k054000->regs[offset] = data;
}

READ8_DEVICE_HANDLER( k054000_r )
{
	k054000_state *k054000 = k054000_get_safe_token(device);
	int Acx, Acy, Aax, Aay;
	int Bcx, Bcy, Bax, Bay;

	//logerror("%04x: read 054000 address %02x\n", cpu_get_pc(space->cpu), offset);

	if (offset != 0x18)
		return 0;

	Acx = (k054000->regs[0x01] << 16) | (k054000->regs[0x02] << 8) | k054000->regs[0x03];
	Acy = (k054000->regs[0x09] << 16) | (k054000->regs[0x0a] << 8) | k054000->regs[0x0b];

	/* TODO: this is a hack to make thndrx2 pass the startup check. It is certainly wrong. */
	if (k054000->regs[0x04] == 0xff)
		Acx+=3;
	if (k054000->regs[0x0c] == 0xff)
		Acy+=3;

	Aax = k054000->regs[0x06] + 1;
	Aay = k054000->regs[0x07] + 1;

	Bcx = (k054000->regs[0x15] << 16) | (k054000->regs[0x16] << 8) | k054000->regs[0x17];
	Bcy = (k054000->regs[0x11] << 16) | (k054000->regs[0x12] << 8) | k054000->regs[0x13];
	Bax = k054000->regs[0x0e] + 1;
	Bay = k054000->regs[0x0f] + 1;

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

READ16_DEVICE_HANDLER( k054000_lsb_r )
{
	return k054000_r(device, offset);
}

WRITE16_DEVICE_HANDLER( k054000_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		k054000_w(device, offset, data & 0xff);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k054000 )
{
	k054000_state *k054000 = k054000_get_safe_token(device);

	state_save_register_device_item_array(device, 0, k054000->regs);
}

static DEVICE_RESET( k054000 )
{
	k054000_state *k054000 = k054000_get_safe_token(device);
	int i;

	for (i = 0; i < 0x20; i++)
		k054000->regs[i] = 0;
}


/***************************************************************************/
/*                                                                         */
/*                                 051733                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k051733_state k051733_state;
struct _k051733_state
{
	UINT8    ram[0x20];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k051733_state *k051733_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K051733);

	return (k051733_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( k051733_w )
{
	k051733_state *k051733= k051733_get_safe_token(device);
	//logerror("%04x: write %02x to 051733 address %02x\n", cpu_get_pc(space->cpu), data, offset);

	k051733->ram[offset] = data;
}


static int k051733_int_sqrt( UINT32 op )
{
	UINT32 i = 0x8000;
	UINT32 step = 0x4000;

	while (step)
	{
		if (i * i == op)
			return i;
		else if (i * i > op)
			i -= step;
		else
			i += step;
		step >>= 1;
	}
	return i;
}

READ8_DEVICE_HANDLER( k051733_r )
{
	k051733_state *k051733= k051733_get_safe_token(device);

	int op1 = (k051733->ram[0x00] << 8) | k051733->ram[0x01];
	int op2 = (k051733->ram[0x02] << 8) | k051733->ram[0x03];
	int op3 = (k051733->ram[0x04] << 8) | k051733->ram[0x05];

	int rad = (k051733->ram[0x06] << 8) | k051733->ram[0x07];
	int yobj1c = (k051733->ram[0x08] << 8) | k051733->ram[0x09];
	int xobj1c = (k051733->ram[0x0a] << 8) | k051733->ram[0x0b];
	int yobj2c = (k051733->ram[0x0c] << 8) | k051733->ram[0x0d];
	int xobj2c = (k051733->ram[0x0e] << 8) | k051733->ram[0x0f];

	//logerror("%04x: read 051733 address %02x\n", cpu_get_pc(space->cpu), offset);

	switch (offset)
	{
		case 0x00:
			if (op2)
				return (op1 / op2) >> 8;
			else
				return 0xff;
		case 0x01:
			if (op2)
				return (op1 / op2) & 0xff;
			else
				return 0xff;

		/* this is completely unverified */
		case 0x02:
			if (op2)
				return (op1 % op2) >> 8;
			else
				return 0xff;
		case 0x03:
			if (op2)
				return (op1 % op2) & 0xff;
			else
				return 0xff;

		case 0x04:
			return k051733_int_sqrt(op3 << 16) >> 8;

		case 0x05:
			return k051733_int_sqrt(op3 << 16) & 0xff;

		case 0x06:
			return k051733->ram[0x13];

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
			return ~k051733->ram[offset];
		case 0x0f:
			return ~k051733->ram[offset];
		default:
			return k051733->ram[offset];
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k051733 )
{
	k051733_state *k051733 = k051733_get_safe_token(device);

	state_save_register_device_item_array(device, 0, k051733->ram);
}

static DEVICE_RESET( k051733 )
{
	k051733_state *k051733 = k051733_get_safe_token(device);
	int i;

	for (i = 0; i < 0x20; i++)
		k051733->ram[i] = 0;
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

typedef struct _k056832_state k056832_state;
struct _k056832_state
{
	tilemap_t   *tilemap[K056832_PAGE_COUNT];
	bitmap_t  *pixmap[K056832_PAGE_COUNT];

	UINT16    regs[0x20];	// 157/832 regs group 1
	UINT16    regsb[4];	// 157/832 regs group 2, board dependent

	UINT8 *   rombase;	// pointer to tile gfx data
	UINT16 *  videoram;
	int       num_gfx_banks;	// depends on size of graphics ROMs
	int       cur_gfx_banks;		// cached info for K056832_regs[0x1a]
	int       gfxnum;			// graphics element index for unpacked tiles
	const char *memory_region;	// memory region for tile gfx data


	// ROM readback involves reading 2 halves of a word
	// from the same location in a row.  Reading the
	// RAM window resets this state so you get the first half.
	int       rom_half;


	// locally cached values
	int       layer_assoc_with_page[K056832_PAGE_COUNT];
	int       layer_offs[8][2];
	int       lsram_page[8][2];
	INT32     x[8];	// 0..3 left
	INT32     y[8];	// 0..3 top
	INT32     w[8];	// 0..3 width  -> 1..4 pages
	INT32     h[8];	// 0..3 height -> 1..4 pages
	INT32     dx[8];	// scroll
	INT32     dy[8];	// scroll
	UINT32    line_dirty[K056832_PAGE_COUNT][8];
	UINT8     all_lines_dirty[K056832_PAGE_COUNT];
	UINT8     page_tile_mode[K056832_PAGE_COUNT];
	int       last_colorbase[K056832_PAGE_COUNT];
	UINT8     layer_tile_mode[8];
	int       default_layer_association;
	int       layer_association;
	int       active_layer;
	int       selected_page;
	int       selected_page_x4096;
	int       linemap_enabled;
	int       use_ext_linescroll;
	int       uses_tile_banks, cur_tile_bank;

	int       djmain_hack;

	k056832_callback  callback;

	const device_config *k055555;	/* used to choose colorbase */
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k056832_state *k056832_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K056832);

	return (k056832_state *)device->token;
}

INLINE const k056832_interface *k056832_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert(device->type == K056832);
	return (const k056832_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

#define k056832_mark_line_dirty(P, L) if (L < 0x100) k056832->line_dirty[P][L >> 5] |= 1 << (L & 0x1f)
#define k056832_mark_all_lines_dirty(P) k056832->all_lines_dirty[P] = 1

static void k056832_mark_page_dirty( k056832_state *k056832, int page )
{
	if (k056832->page_tile_mode[page])
		tilemap_mark_all_tiles_dirty(k056832->tilemap[page]);
	else
		k056832_mark_all_lines_dirty(page);
}

void k056832_mark_plane_dirty( const device_config *device, int layer )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int tilemode, i;

	tilemode = k056832->layer_tile_mode[layer];

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		if (k056832->layer_assoc_with_page[i] == layer)
		{
			k056832->page_tile_mode[i] = tilemode;
			k056832_mark_page_dirty(k056832, i);
		}
	}
}

static void k056832_mark_all_tilemaps_dirty( k056832_state *k056832 )
{
	int i;

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		if (k056832->layer_assoc_with_page[i] != -1)
		{
			k056832->page_tile_mode[i] = k056832->layer_tile_mode[k056832->layer_assoc_with_page[i]];
			k056832_mark_page_dirty(k056832, i);
		}
	}
}

/* moo.c needs to call this in its VIDEO_UPDATE */
void k056832_mark_all_tmaps_dirty( const device_config *device )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832_mark_all_tilemaps_dirty(k056832);
}

static void k056832_update_page_layout( k056832_state *k056832 )
{
	int layer, rowstart, rowspan, colstart, colspan, r, c, page_idx, setlayer;

	// enable layer association by default
	k056832->layer_association = k056832->default_layer_association;

	// disable association if a layer grabs the entire 4x4 map (happens in Twinbee and Dadandarn)
	for (layer = 0; layer < 4; layer++)
	{
		if (!k056832->y[layer] && !k056832->x[layer] && k056832->h[layer] == 3 && k056832->w[layer] == 3)
		{
			k056832->layer_association = 0;
			break;
		}
	}

	// winning spike doesn't like layer association..
	if (k056832->djmain_hack == 2)
		k056832->layer_association = 0;

	// disable all tilemaps
	for (page_idx = 0; page_idx < K056832_PAGE_COUNT; page_idx++)
	{
		k056832->layer_assoc_with_page[page_idx] = -1;
	}


	// enable associated tilemaps
	for (layer = 0; layer < 4; layer++)
	{
		rowstart = k056832->y[layer];
		colstart = k056832->x[layer];
		rowspan  = k056832->h[layer] + 1;
		colspan  = k056832->w[layer] + 1;

		setlayer = (k056832->layer_association) ? layer : k056832->active_layer;

		for (r = 0; r < rowspan; r++)
		{
			for (c = 0; c < colspan; c++)
			{
				page_idx = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);
				if (!(k056832->djmain_hack==1) || k056832->layer_assoc_with_page[page_idx] == -1)
					k056832->layer_assoc_with_page[page_idx] = setlayer;
			}
		}
	}

	// refresh associated tilemaps
	k056832_mark_all_tilemaps_dirty(k056832);
}

int k056832_get_lookup( const device_config *device, int bits )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int res;

	res = (k056832->regs[0x1c] >> (bits << 2)) & 0x0f;

	if (k056832->uses_tile_banks)	/* Asterix */
		res |= k056832->cur_tile_bank << 4;

	return res;
}

INLINE void k056832_get_tile_info( const device_config *device, tile_data *tileinfo, int tile_index, int pageIndex )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	static const struct K056832_SHIFTMASKS
	{
		int flips, palm1, pals2, palm2;
	}
	k056832_shiftmasks[4] = {{6, 0x3f, 0, 0x00}, {4, 0x0f, 2, 0x30}, {2, 0x03, 2, 0x3c}, {0, 0x00, 2, 0x3f}};

	const struct K056832_SHIFTMASKS *smptr;
	int layer, flip, fbits, attr, code, color, flags;
	UINT16 *pMem;

	pMem  = &k056832->videoram[(pageIndex << 12) + (tile_index << 1)];

	if (k056832->layer_association)
	{
		layer = k056832->layer_assoc_with_page[pageIndex];
		if (layer == -1)
			layer = 0;	// use layer 0's palette info for unmapped pages
	}
	else
		layer = k056832->active_layer;

	fbits = (k056832->regs[3] >> 6) & 3;
	flip  = (k056832->regs[1] >> (layer << 1)) & 0x3; // tile-flip override (see p.20 3.2.2 "REG2")
	smptr = &k056832_shiftmasks[fbits];
	attr  = pMem[0];
	code  = pMem[1];

	// normalize the flip/palette flags
	// see the tables on pages 4 and 10 of the Pt. 2-3 "VRAM" manual
	// for a description of these bits "FBIT0" and "FBIT1"
	flip &= attr >> smptr->flips & 3;
	color = (attr & smptr->palm1) | (attr >> smptr->pals2 & smptr->palm2);
	flags = TILE_FLIPYX(flip);

	k056832->callback(device->machine, layer, &code, &color, &flags);

	SET_TILE_INFO_DEVICE(
			k056832->gfxnum,
			code,
			color,
			flags);
}

static TILE_GET_INFO_DEVICE( k056832_get_tile_info0 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x0); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info1 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x1); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info2 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x2); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info3 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x3); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info4 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x4); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info5 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x5); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info6 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x6); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info7 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x7); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info8 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x8); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_info9 ) { k056832_get_tile_info(device, tileinfo, tile_index, 0x9); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infoa ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xa); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infob ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xb); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infoc ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xc); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infod ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xd); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infoe ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xe); }
static TILE_GET_INFO_DEVICE( k056832_get_tile_infof ) { k056832_get_tile_info(device, tileinfo, tile_index, 0xf); }

static void k056832_change_rambank( k056832_state *k056832 )
{
	/* ------xx page col
     * ---xx--- page row
     */
	int bank = k056832->regs[0x19];

	if (k056832->regs[0] & 0x02)	// external linescroll enable
		k056832->selected_page = K056832_PAGE_COUNT;
	else
		k056832->selected_page = ((bank >> 1) & 0xc) | (bank & 3);

	k056832->selected_page_x4096 = k056832->selected_page << 12;

	// refresh associated tilemaps
	k056832_mark_all_tilemaps_dirty(k056832);
}

int k056832_get_current_rambank( const device_config *device )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int bank = k056832->regs[0x19];

	return ((bank >> 1) & 0xc) | (bank & 3);
}

static void k056832_change_rombank( k056832_state *k056832 )
{
	int bank;

	if (k056832->uses_tile_banks)	/* Asterix */
		bank = (k056832->regs[0x1a] >> 8) | (k056832->regs[0x1b] << 4) | (k056832->cur_tile_bank << 6);
	else
		bank = k056832->regs[0x1a] | (k056832->regs[0x1b] << 16);

	k056832->cur_gfx_banks = bank % k056832->num_gfx_banks;
}

void k056832_set_tile_bank( const device_config *device, int bank )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->uses_tile_banks = 1;

	if (k056832->cur_tile_bank != bank)
	{
		k056832->cur_tile_bank = bank;

		k056832_mark_plane_dirty(device, 0);
		k056832_mark_plane_dirty(device, 1);
		k056832_mark_plane_dirty(device, 2);
		k056832_mark_plane_dirty(device, 3);
	}

	k056832_change_rombank(k056832);
}

/* call if a game uses external linescroll */
void k056832_SetExtLinescroll( const device_config *device )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->use_ext_linescroll = 1;
}

/* generic helper routine for ROM checksumming */
static int k056832_rom_read_b( const device_config *device, int offset, int blksize, int blksize2, int zerosec )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT8 *rombase;
	int base, ret;

	rombase = (UINT8 *)memory_region(device->machine, k056832->memory_region);

	if ((k056832->rom_half) && (zerosec))
	{
		return 0;
	}

	// add in the bank offset
	offset += (k056832->cur_gfx_banks * 0x2000);

	// figure out the base of the ROM block
	base = (offset / blksize) * blksize2;

	// get the starting offset of the proper word inside the block
	base += (offset % blksize) * 2;

	if (k056832->rom_half)
	{
		ret = rombase[base + 1];
	}
	else
	{
		ret = rombase[base];
		k056832->rom_half = 1;
	}

	return ret;
}

READ16_DEVICE_HANDLER( k056832_5bpp_rom_word_r )
{
	if (mem_mask == 0xff00)
		return k056832_rom_read_b(device, offset * 2, 4, 5, 0)<<8;
	else if (mem_mask == 0x00ff)
		return k056832_rom_read_b(device, offset * 2 + 1, 4, 5, 0)<<16;
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ32_DEVICE_HANDLER( k056832_5bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
		return k056832_rom_read_b(device, offset * 4, 4, 5, 0) << 24;
	else if (mem_mask == 0x00ff0000)
		return k056832_rom_read_b(device, offset * 4 + 1, 4, 5, 0) << 16;
	else if (mem_mask == 0x0000ff00)
		return k056832_rom_read_b(device, offset * 4 + 2, 4, 5, 0) << 8;
	else if (mem_mask == 0x000000ff)
		return k056832_rom_read_b(device, offset * 4 + 3, 4, 5, 1);
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ32_DEVICE_HANDLER( k056832_6bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
		return k056832_rom_read_b(device, offset * 4, 4, 6, 0) << 24;
	else if (mem_mask == 0x00ff0000)
		return k056832_rom_read_b(device, offset * 4 + 1, 4, 6, 0) << 16;
	else if (mem_mask == 0x0000ff00)
		return k056832_rom_read_b(device, offset * 4 + 2, 4, 6, 0) << 8;
	else if (mem_mask == 0x000000ff)
		return k056832_rom_read_b(device, offset * 4 + 3, 4, 6, 0);
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ16_DEVICE_HANDLER( k056832_rom_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int ofs16, ofs8;
	UINT8 *rombase;
	int ret;

	ofs16 = (offset / 8)*5;
	ofs8 = (offset / 4)*5;

	ofs16 += (k056832->cur_gfx_banks * 5 * 1024);
	ofs8 += (k056832->cur_gfx_banks * 10 * 1024);

	if (!k056832->rombase)
	{
		k056832->rombase = memory_region(device->machine, k056832->memory_region);
	}
	rombase = (UINT8 *)k056832->rombase;

	ret = (rombase[ofs8 + 4]<<8);
	if ((offset % 8) >= 4)
	{
		ret |= (rombase[ofs16 + 1] << 24) | (rombase[ofs16 + 3] << 16);
	}
	else
	{
		ret |= (rombase[ofs16] << 24) | (rombase[ofs16 + 2] << 16);
	}

	return ret;
}

// data is arranged like this:
// 0000 1111 22 0000 1111 22
READ16_DEVICE_HANDLER( k056832_mw_rom_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int bank = 10240 * k056832->cur_gfx_banks;
	int addr;

	if (!k056832->rombase)
		k056832->rombase = memory_region(device->machine, k056832->memory_region);

	if (k056832->regsb[2] & 0x8)
	{
		// we want only the 2s
		int bit;
		int res, temp;

		bit = offset % 4;
		addr = (offset / 4) * 5;

		temp = k056832->rombase[addr + 4 + bank];

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

		addr = (offset >> 1) * 5;

		if (offset & 1)
		{
			addr += 2;
		}

		addr += bank;

		return k056832->rombase[addr + 1] | (k056832->rombase[addr] << 8);
	}

}

READ16_DEVICE_HANDLER( k056832_bishi_rom_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int addr = 0x4000 * k056832->cur_gfx_banks + offset;

	if (!k056832->rombase)
		k056832->rombase = memory_region(device->machine, k056832->memory_region);

	return k056832->rombase[addr + 2] | (k056832->rombase[addr] << 8);
}

READ16_DEVICE_HANDLER( k056832_rom_word_8000_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int addr = 0x8000 * k056832->cur_gfx_banks + 2 * offset;

	if (!k056832->rombase)
		k056832->rombase = memory_region(device->machine, k056832->memory_region);

	return k056832->rombase[addr + 2] | (k056832->rombase[addr] << 8);
}

READ16_DEVICE_HANDLER( k056832_old_rom_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int addr = 0x2000 * k056832->cur_gfx_banks + 2 * offset;

	if (!k056832->rombase)
		k056832->rombase = memory_region(device->machine, k056832->memory_region);

	return k056832->rombase[addr + 1] | (k056832->rombase[addr] << 8);
}

READ32_DEVICE_HANDLER( k056832_rom_long_r )
{
	offset <<= 1;
	return (k056832_rom_word_r(device, offset + 1, 0xffff) | (k056832_rom_word_r(device, offset, 0xffff) << 16));
}

/* only one page is mapped to videoram at a time through a window */
READ16_DEVICE_HANDLER( k056832_ram_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);

	// reading from tile RAM resets the ROM readback "half" offset
	k056832->rom_half = 0;

	return k056832->videoram[k056832->selected_page_x4096 + offset];
}

READ16_DEVICE_HANDLER( k056832_ram_half_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return k056832->videoram[k056832->selected_page_x4096 + (((offset << 1) & 0xffe) | ((offset >> 11) ^ 1))];
}

READ32_DEVICE_HANDLER( k056832_ram_long_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *pMem = &k056832->videoram[k056832->selected_page_x4096 + offset * 2];

	// reading from tile RAM resets the ROM readback "half" offset
	k056832->rom_half = 0;

	return (pMem[0]<<16 | pMem[1]);
}

/* special 8-bit handlers for Lethal Enforcers */
READ8_DEVICE_HANDLER( k056832_ram_code_lo_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2) + 1];

	return *adr & 0xff;
}

READ8_DEVICE_HANDLER( k056832_ram_code_hi_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2) + 1];

	return *adr >> 8;
}

READ8_DEVICE_HANDLER( k056832_ram_attr_lo_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2)];

	return *adr & 0xff;
}

READ8_DEVICE_HANDLER( k056832_ram_attr_hi_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2)];

	return *adr >> 8;
}

WRITE8_DEVICE_HANDLER( k056832_ram_code_lo_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2) + 1];

	*adr &= 0xff00;
	*adr |= data;

	if (!(k056832->regs[0] & 0x02))	// external linescroll enable
	{
		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE8_DEVICE_HANDLER( k056832_ram_code_hi_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2) + 1];

	*adr &= 0x00ff;
	*adr |= data << 8;

	if (!(k056832->regs[0] & 0x02))	// external linescroll enable
	{
		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE8_DEVICE_HANDLER( k056832_ram_attr_lo_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2)];

	*adr &= 0xff00;
	*adr |= data;

	if (!(k056832->regs[0] & 0x02))	// external linescroll enable
	{
		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE8_DEVICE_HANDLER( k056832_ram_attr_hi_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (offset * 2)];

	*adr &= 0x00ff;
	*adr |= data << 8;

	if (!(k056832->regs[0] & 0x02))	// external linescroll enable
	{
		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE16_DEVICE_HANDLER( k056832_ram_word_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *tile_ptr;
	UINT16 old_mask, old_data;

	tile_ptr = &k056832->videoram[k056832->selected_page_x4096 + offset];
	old_mask = ~mem_mask;
	old_data = *tile_ptr;
	data = (data & mem_mask) | (old_data & old_mask);

	if(data != old_data)
	{
		offset >>= 1;
		*tile_ptr = data;

		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE16_DEVICE_HANDLER( k056832_ram_half_word_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *adr = &k056832->videoram[k056832->selected_page_x4096 + (((offset << 1) & 0xffe) | 1)];
	UINT16 old = *adr;

	COMBINE_DATA(adr);
	if(*adr != old)
	{
		int dofs = (((offset << 1) & 0xffe) | 1);

		dofs >>= 1;

		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], dofs);
		else
    		k056832_mark_line_dirty(k056832->selected_page, dofs);
	}
}

WRITE32_DEVICE_HANDLER( k056832_ram_long_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT16 *tile_ptr;
	UINT32 old_mask, old_data;

	tile_ptr = &k056832->videoram[k056832->selected_page_x4096 + offset * 2];
	old_mask = ~mem_mask;
	old_data = (UINT32)tile_ptr[0] << 16 | (UINT32)tile_ptr[1];
	data = (data & mem_mask) | (old_data & old_mask);

	if (data != old_data)
	{
		tile_ptr[0] = data >> 16;
		tile_ptr[1] = data;

		if (k056832->page_tile_mode[k056832->selected_page])
			tilemap_mark_tile_dirty(k056832->tilemap[k056832->selected_page], offset);
		else
			k056832_mark_line_dirty(k056832->selected_page, offset);
	}
}

WRITE16_DEVICE_HANDLER( k056832_word_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	int layer, flip, mask, i;
	UINT32 old_data, new_data;

	old_data = k056832->regs[offset];
	COMBINE_DATA(&k056832->regs[offset]);
	new_data = k056832->regs[offset];

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
					for (i = 0; i < K056832_PAGE_COUNT; i++)
					{
						tilemap_set_flip(k056832->tilemap[i], flip);
					}
				}

				if ((new_data & 0x02) != (old_data & 0x02))
				{
					k056832_change_rambank(k056832);
				}
			break;

			/* -------- -----xxx external irqlines enable (not used by GX)
             * -------- xx------ tilemap attribute config (FBIT0 and FBIT1)
             */
			//case 0x06/2: break;

			// -------- ----DCBA tile mode: 0=512x1, 1=8x8
			// -------- DCBA---- synchronous scroll: 0=off, 1=on
			case 0x08/2:
				for (layer = 0; layer < 4; layer++)
				{
					mask = 1 << layer;
					i = new_data & mask;
					if (i != (old_data & mask))
					{
						k056832->layer_tile_mode[layer] = i;
						k056832_mark_plane_dirty(device, layer);
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
				k056832_change_rambank(k056832);
			break;

			case 0x34/2: /* ROM bank select for checksum */
			case 0x36/2: /* secondary ROM bank select for use with tile banking */
				k056832_change_rombank(k056832);
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
					k056832->y[layer] = (new_data & 0x18) >> 3;
					k056832->h[layer] = (new_data & 0x3);
					k056832->active_layer = layer;
					k056832_update_page_layout(k056832);
				} else

				if (offset >= 0x18/2 && offset <= 0x1e/2)
				{
					k056832->x[layer] = (new_data & 0x18) >> 3;
					k056832->w[layer] = (new_data & 0x03);
					k056832->active_layer = layer;
					k056832_update_page_layout(k056832);
				} else

				if (offset >= 0x20/2 && offset <= 0x26/2)
				{
					k056832->dy[layer] = (INT16)new_data;
				} else

				if (offset >= 0x28/2 && offset <= 0x2e/2)
				{
					k056832->dx[layer] = (INT16)new_data;
				}
			break;
		}
	}
}

WRITE32_DEVICE_HANDLER( k056832_long_w )
{
	// GX does access of all 3 widths (8/16/32) so we can't do the
	// if (ACCESSING_xxx) trick.  in particular, 8-bit writes
	// are used to the tilemap bank register.
	offset <<= 1;
	k056832_word_w(device, offset, data >> 16, mem_mask >> 16);
	k056832_word_w(device, offset + 1, data, mem_mask);
}

WRITE16_DEVICE_HANDLER( k056832_b_word_w )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	COMBINE_DATA(&k056832->regsb[offset]);
}

WRITE8_DEVICE_HANDLER( k056832_w )
{
	if (offset & 1)
	{
		k056832_word_w(device, (offset >> 1), data, 0x00ff);
	}
	else
	{
		k056832_word_w(device, (offset >> 1), data << 8, 0xff00);
	}
}

WRITE8_DEVICE_HANDLER( k056832_b_w )
{
	if (offset & 1)
	{
		k056832_b_word_w(device, (offset >> 1), data, 0x00ff);
	}
	else
	{
		k056832_b_word_w(device, (offset >> 1), data<<8, 0xff00);
	}
}

WRITE32_DEVICE_HANDLER( k056832_b_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		k056832_b_word_w(device, offset << 1, data >> 16, mem_mask >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		k056832_b_word_w(device, (offset << 1) + 1, data, mem_mask);
	}
}

static int k056832_update_linemap( const device_config *device, bitmap_t *bitmap, int page, int flags )
{
	k056832_state *k056832 = k056832_get_safe_token(device);

	if (k056832->page_tile_mode[page])
		return(0);
	if (!k056832->linemap_enabled)
		return(1);

	{
		rectangle zerorect;
		tilemap_t *tmap;
		UINT32 *dirty;
		int all_dirty;
		bitmap_t* xprmap;
		UINT8 *xprdata;

		tmap = k056832->tilemap[page];
		xprmap  = tilemap_get_flagsmap(tmap);
		xprdata = tilemap_get_tile_flags(tmap);

		dirty = k056832->line_dirty[page];
		all_dirty = k056832->all_lines_dirty[page];

		if (all_dirty)
		{
			dirty[7] = dirty[6] = dirty[5] = dirty[4] = dirty[3] = dirty[2] = dirty[1] = dirty[0] = 0;
			k056832->all_lines_dirty[page] = 0;

			// force tilemap into a clean, static state
			// *really ugly but it minimizes alteration to tilemap.c
			memset(&zerorect, 0, sizeof(rectangle));	// zero dimension
			tilemap_draw(bitmap, &zerorect, tmap, 0, 0);	// dummy call to reset tile_dirty_map
			bitmap_fill(xprmap, 0, 0);						// reset pixel transparency_bitmap;
			memset(xprdata, TILEMAP_PIXEL_LAYER0, 0x800);	// reset tile transparency_data;
		}
		else
		{
			if (!(dirty[0] | dirty[1] | dirty[2] | dirty[3] | dirty[4] | dirty[5] | dirty[6] | dirty[7]))
				return 0;
		}

#if 0	/* this code is broken.. really broken .. gijoe uses it for some line/column scroll style effects (lift level of attract mode)
            we REALLY shouldn't be writing directly back into the pixmap, surely this should
            be done when rendering instead

        */
		{

			bitmap_t *pixmap;
			running_machine *machine = device->machine;

			UINT8 code_transparent, code_opaque;
			const pen_t *pal_ptr;
			const UINT8  *src_ptr;
			UINT8  *xpr_ptr;
			UINT16 *dst_ptr;
			UINT16 pen, basepen;
			int count, src_pitch, src_modulo;
			int dst_pitch;
			int line;
			const gfx_element *src_gfx;
			int offs, mask;

			#define LINE_WIDTH 512

			#define DRAW_PIX(N) \
				pen = src_ptr[N]; \
				if (pen) \
				{ pen += basepen; xpr_ptr[count+N] = TILEMAP_PIXEL_LAYER0; dst_ptr[count+N] = pen; } else \
				{ xpr_ptr[count+N] = 0; }

			pixmap  = k056832->pixmap[page];
			pal_ptr = machine->pens;
			src_gfx = machine->gfx[k056832->gfxnum];
			src_pitch  = src_gfx->line_modulo;
			src_modulo = src_gfx->char_modulo;
			dst_pitch  = pixmap->rowpixels;

			for (line = 0; line < 256; line++)
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

				for (count = 0; count < LINE_WIDTH; count += 8)
				{
					k056832_get_tile_info(device, &tileinfo, line, page);
					basepen = tileinfo.palette_base;
					code_transparent = tileinfo.category;
					code_opaque = code_transparent | TILEMAP_PIXEL_LAYER0;

					src_ptr = tileinfo.pen_data + count * 8;//src_base + ((tileinfo.tile_number & ~7) << 6);

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

void k056832_tilemap_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, UINT32 flags, UINT32 priority )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap_t *tmap;
	UINT16 *p_scroll_data;
	UINT16 ram16[2];

	int rowstart = k056832->y[layer];
	int colstart = k056832->x[layer];
	int rowspan  = k056832->h[layer] + 1;
	int colspan  = k056832->w[layer] + 1;
	int dy = k056832->dy[layer];
	int dx = k056832->dx[layer];
	int scrollbank = ((k056832->regs[0x18] >> 1) & 0xc) | (k056832->regs[0x18] & 3);
	int scrollmode = k056832->regs[0x05] >> (k056832->lsram_page[layer][0] << 1) & 3;

	if (k056832->use_ext_linescroll)
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
	flipy = k056832->regs[0] & 0x20;
	if (flipy)
	{
		corr = k056832->regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	}
	else
		corr = 0;

	dy += corr;
	ay = (unsigned)(dy - k056832->layer_offs[layer][1]) % height;

	flipx = k056832->regs[0] & 0x10;
	if (flipx)
	{
		corr = k056832->regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	}
	else
		corr = 0;

	corr -= k056832->layer_offs[layer][0];

	if (scrollmode == 0 && (flags & K056832_DRAW_FLAG_FORCE_XYSCROLL))
	{
		scrollmode = 3;
		flags &= ~K056832_DRAW_FLAG_FORCE_XYSCROLL;
	}

	switch( scrollmode )
	{
		case 0: // linescroll
			p_scroll_data = &k056832->videoram[scrollbank<<12] + (k056832->lsram_page[layer][1]>>1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll

			p_scroll_data = &k056832->videoram[scrollbank << 12] + (k056832->lsram_page[layer][1] >> 1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			p_scroll_data = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy)
		sdat_adv = -sdat_adv;

	last_active = k056832->active_layer;
	new_colorbase = (k056832->k055555 != NULL) ? k055555_get_palette_index(k056832->k055555, layer) : 0;

	for (r = 0; r < rowspan; r++)
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
				if ((r == rowspan - 1) && (sy < K056832_PAGE_HEIGHT)) sy += height;
				if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

				// switch frame of reference and clip y
				if ((ty -= sy) <= 0)
				{
					cliph = K056832_PAGE_HEIGHT + ty;
					clipy = line_starty = -ty;
					line_endy = K056832_PAGE_HEIGHT;
					sdat_start = K056832_PAGE_HEIGHT - 1;
					if (scrollmode == 2) sdat_start &= ~7;
				}
				else
				{
					cliph = K056832_PAGE_HEIGHT - ty;
					clipy = line_starty = 0;
					line_endy = cliph;
					sdat_start = cliph - 1;
					if (scrollmode == 2)
					{
						sdat_start &= ~7;
						line_starty -= ty & 7;
					}
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
			sdat_start = K056832_PAGE_HEIGHT - 1;

			if (scrollmode == 2) { sdat_start &= ~7; line_starty -= dy & 7; }
		}

		sdat_start += r * K056832_PAGE_HEIGHT;
		sdat_start <<= 1;

		clipmaxy = clipy + cliph - 1;

		for (c = 0; c < colspan; c++)
		{
			pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

			if (k056832->layer_association)
			{
				if (k056832->layer_assoc_with_page[pageIndex] != layer)
					continue;
			}
			else
			{
				if (k056832->layer_assoc_with_page[pageIndex] == -1)
					continue;

				k056832->active_layer = layer;
			}

			if (k056832->k055555 != NULL)		// are we using k055555 palette?
			{
				if (k056832->last_colorbase[pageIndex] != new_colorbase)
				{
					k056832->last_colorbase[pageIndex] = new_colorbase;
					k056832_mark_page_dirty(k056832, pageIndex);
				}
			}
			else
			{
				if (!pageIndex)
					k056832->active_layer = 0;
			}

			if (k056832_update_linemap(device, bitmap, pageIndex, flags))
				continue;

			tmap = k056832->tilemap[pageIndex];
			tilemap_set_scrolly(tmap, 0, ay);

			last_dx = 0x100000;
			last_visible = 0;

			for (sdat_walk = sdat_start, line_y = line_starty; line_y < line_endy; sdat_walk += sdat_adv, line_y += line_height)
			{
				dminy = line_y;
				dmaxy = line_y + line_height - 1;

				if (dminy < clipy) dminy = clipy;
				if (dmaxy > clipmaxy) dmaxy = clipmaxy;
				if (dminy > cmaxy || dmaxy < cminy) continue;

				sdat_offs = sdat_walk & sdat_wrapmask;

				drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
				drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

				dx = ((int)p_scroll_data[sdat_offs]<<16 | (int)p_scroll_data[sdat_offs + 1]) + corr;

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
				if ((drawrect.min_x>0) && (drawrect.max_x==511))
					drawrect.max_x=cliprect->max_x;

				tilemap_set_scrollx(tmap, 0, dx);

				LINE_SHORTCIRCUIT:
					tilemap_draw(bitmap, &drawrect, tmap, flags, priority);

			} // end of line loop
		} // end of column loop
	} // end of row loop

	k056832->active_layer = last_active;
} // end of function


void k056832_tilemap_draw_dj( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, UINT32 flags, UINT32 priority )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap_t *tmap;
	UINT16 *p_scroll_data;
	UINT16 ram16[2];

	int rowstart = k056832->y[layer];
	int colstart = k056832->x[layer];
	int rowspan  = k056832->h[layer] + 1;
	int colspan  = k056832->w[layer] + 1;
	int dy = k056832->dy[layer];
	int dx = k056832->dx[layer];
	int scrollbank = ((k056832->regs[0x18] >> 1) & 0xc) | (k056832->regs[0x18] & 3);
	int scrollmode = k056832->regs[0x05] >> (k056832->lsram_page[layer][0] << 1) & 3;
	int need_wrap = -1;

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect->min_x;
	cmaxx = cliprect->max_x;
	cminy = cliprect->min_y;
	cmaxy = cliprect->max_y;

	// flip correction registers
	flipy = k056832->regs[0] & 0x20;
	if (flipy)
	{
		corr = k056832->regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	}
	else
		corr = 0;
	dy += corr;
	ay = (unsigned)(dy - k056832->layer_offs[layer][1]) % height;

	flipx = k056832->regs[0] & 0x10;
	if (flipx)
	{
		corr = k056832->regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	}
	else
		corr = 0;

	corr -= k056832->layer_offs[layer][0];

	if (scrollmode == 0 && (flags & K056832_DRAW_FLAG_FORCE_XYSCROLL))
	{
		scrollmode = 3;
		flags &= ~K056832_DRAW_FLAG_FORCE_XYSCROLL;
	}

	switch( scrollmode )
	{
		case 0: // linescroll
			p_scroll_data = &k056832->videoram[scrollbank << 12] + (k056832->lsram_page[layer][1] >> 1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll
			p_scroll_data = &k056832->videoram[scrollbank << 12] + (k056832->lsram_page[layer][1] >> 1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			p_scroll_data = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy)
		sdat_adv = -sdat_adv;

	last_active = k056832->active_layer;
	new_colorbase = (k056832->k055555 != NULL) ? k055555_get_palette_index(k056832->k055555, layer) : 0;

	for (r = 0; r <= rowspan; r++)
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
		if ((sy + height <= ty) || (sy - height >= ty))
			continue;

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

		for (c = 0; c < colspan; c++)
		{
			if (r == rowspan)
				pageIndex = (((rowstart + need_wrap) & 3) << 2) + ((colstart + c) & 3);
			else
				pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

			if (k056832->layer_association)
			{
				if (k056832->layer_assoc_with_page[pageIndex] != layer)
					continue;
			}
			else
			{
				if (k056832->layer_assoc_with_page[pageIndex] == -1) continue;
				k056832->active_layer = layer;
			}

			if (k056832->k055555 != NULL)		// are we using k055555 palette?
			{
				if (k056832->last_colorbase[pageIndex] != new_colorbase)
				{
					k056832->last_colorbase[pageIndex] = new_colorbase;
					k056832_mark_page_dirty(k056832, pageIndex);
				}
			}
			else
			{
				if (!pageIndex)
					k056832->active_layer = 0;
			}

			if (k056832_update_linemap(device, bitmap, pageIndex, flags))
				continue;

			tmap = k056832->tilemap[pageIndex];
			tilemap_set_scrolly(tmap, 0, ay);

			last_dx = 0x100000;
			last_visible = 0;

			for (sdat_walk = sdat_start, line_y = line_starty; line_y < line_endy; sdat_walk += sdat_adv, line_y += line_height)
			{
				dminy = line_y;
				dmaxy = line_y + line_height - 1;

				if (dminy < clipy) dminy = clipy;
				if (dmaxy > clipmaxy) dmaxy = clipmaxy;
				if (dminy > cmaxy || dmaxy < cminy) continue;

				sdat_offs = sdat_walk & sdat_wrapmask;

				drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
				drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

				dx = ((int)p_scroll_data[sdat_offs] << 16 | (int)p_scroll_data[sdat_offs + 1]) + corr;

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
				else
				{
					clipw = K056832_PAGE_WIDTH;
					clipx = 0;
				}

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

	k056832->active_layer = last_active;

} // end of function


void k056832_set_layer_association( const device_config *device, int status )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->default_layer_association = status;
}

int k056832_get_layer_association( const device_config *device )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return(k056832->layer_association);
}

void k056832_set_layer_offs( const device_config *device, int layer, int offsx, int offsy )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->layer_offs[layer][0] = offsx;
	k056832->layer_offs[layer][1] = offsy;
}

void k056832_set_lsram_page( const device_config *device, int logical_page, int physical_page, int physical_offset )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->lsram_page[logical_page][0] = physical_page;
	k056832->lsram_page[logical_page][1] = physical_offset;
}

void k056832_linemap_enable( const device_config *device, int enable )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	k056832->linemap_enabled = enable;
}

int k056832_is_irq_enabled( const device_config *device, int irqline )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return(k056832->regs[0x06/2] & (1 << irqline & 7));
}

void k056832_read_avac( const device_config *device, int *mode, int *data )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	*mode = k056832->regs[0x04/2] & 7;
	*data = k056832->regs[0x38/2];
}

int k056832_read_register( const device_config *device, int regnum )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return(k056832->regs[regnum]);
}

static STATE_POSTLOAD( k056832_postload )
{
	k056832_state *k056832 = (k056832_state *)param;

	k056832_update_page_layout(k056832);
	k056832_change_rambank(k056832);
	k056832_change_rombank(k056832);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

/* TODO: understand which elements MUST be init here (to keep correct layer
   associations) and which ones can can be init at RESET, if any */
static DEVICE_START( k056832 )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	const k056832_interface *intf = k056832_get_interface(device);
	running_machine *machine = device->machine;
	tilemap_t *tmap;
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


	/* handle the various graphics formats */
	i = (intf->big) ? 8 : 16;


	/* decode the graphics */
	switch (intf->bpp)
	{
		case K056832_BPP_4:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 4);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout4, 4);
			break;

		case K056832_BPP_5:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 5);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout5, 4);
			break;

		case K056832_BPP_6:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 6);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout6, 4);
			break;

		case K056832_BPP_8:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 8);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout8, 4);
			break;

		case K056832_BPP_8LE:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 8);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout8le, 4);
			break;

		case K056832_BPP_4dj:
			total = memory_region_length(machine, intf->gfx_memory_region) / (i * 4);
			decode_gfx(machine, intf->gfx_num, memory_region(machine, intf->gfx_memory_region), total, &charlayout4dj, 4);
			break;

		default:
			fatalerror("Unsupported bpp");
	}

	machine->gfx[intf->gfx_num]->color_granularity = 16; /* override */

	/* deinterleave the graphics, if needed */
	deinterleave_gfx(machine, intf->gfx_memory_region, intf->deinterleave);

	k056832->memory_region = intf->gfx_memory_region;
	k056832->gfxnum = intf->gfx_num;
	k056832->callback = intf->callback;

	k056832->rombase = memory_region(machine, intf->gfx_memory_region);
	k056832->num_gfx_banks = memory_region_length(machine, intf->gfx_memory_region) / 0x2000;
	k056832->djmain_hack = intf->djmain_hack;

	k056832->cur_gfx_banks = 0;
	k056832->use_ext_linescroll = 0;
	k056832->uses_tile_banks = 0;

	for (i = 0; i < 4; i++)
	{
		k056832->layer_offs[i][0] = 0;
		k056832->layer_offs[i][1] = 0;
		k056832->lsram_page[i][0] = i;
		k056832->lsram_page[i][1] = i << 11;
		k056832->x[i] = 0;
		k056832->y[i] = 0;
		k056832->w[i] = 0;
		k056832->h[i] = 0;
		k056832->dx[i] = 0;
		k056832->dy[i] = 0;
		k056832->layer_tile_mode[i] = 1;
	}

	k056832->default_layer_association = 1;
	k056832->active_layer = 0;
	k056832->linemap_enabled = 0;

	k056832->k055555 = devtag_get_device(device->machine, intf->k055555);

	memset(k056832->line_dirty, 0, sizeof(UINT32) * K056832_PAGE_COUNT * 8);

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		k056832->all_lines_dirty[i] = 0;
		k056832->page_tile_mode[i] = 1;
	}

	k056832->videoram = auto_alloc_array(machine, UINT16, 0x2000 * (K056832_PAGE_COUNT + 1) / 2);

	k056832->tilemap[0x0] = tilemap_create_device(device, k056832_get_tile_info0, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x1] = tilemap_create_device(device, k056832_get_tile_info1, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x2] = tilemap_create_device(device, k056832_get_tile_info2, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x3] = tilemap_create_device(device, k056832_get_tile_info3, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x4] = tilemap_create_device(device, k056832_get_tile_info4, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x5] = tilemap_create_device(device, k056832_get_tile_info5, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x6] = tilemap_create_device(device, k056832_get_tile_info6, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x7] = tilemap_create_device(device, k056832_get_tile_info7, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x8] = tilemap_create_device(device, k056832_get_tile_info8, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0x9] = tilemap_create_device(device, k056832_get_tile_info9, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xa] = tilemap_create_device(device, k056832_get_tile_infoa, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xb] = tilemap_create_device(device, k056832_get_tile_infob, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xc] = tilemap_create_device(device, k056832_get_tile_infoc, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xd] = tilemap_create_device(device, k056832_get_tile_infod, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xe] = tilemap_create_device(device, k056832_get_tile_infoe, tilemap_scan_rows,  8, 8, 64, 32);
	k056832->tilemap[0xf] = tilemap_create_device(device, k056832_get_tile_infof, tilemap_scan_rows,  8, 8, 64, 32);

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		tmap = k056832->tilemap[i];

		k056832->pixmap[i] = tilemap_get_pixmap(tmap);

		tilemap_set_transparent_pen(tmap, 0);
	}

	memset(k056832->videoram, 0x00, 0x20000);
	memset(k056832->regs,     0x00, sizeof(k056832->regs) );
	memset(k056832->regsb,    0x00, sizeof(k056832->regsb) );

	k056832_update_page_layout(k056832);

	k056832_change_rambank(k056832);
	k056832_change_rombank(k056832);

	state_save_register_device_item_pointer(device, 0, k056832->videoram, 0x10000);
	state_save_register_device_item_array(device, 0, k056832->regs);
	state_save_register_device_item_array(device, 0, k056832->regsb);
	state_save_register_device_item_array(device, 0, k056832->x);
	state_save_register_device_item_array(device, 0, k056832->y);
	state_save_register_device_item_array(device, 0, k056832->w);
	state_save_register_device_item_array(device, 0, k056832->h);
	state_save_register_device_item_array(device, 0, k056832->dx);
	state_save_register_device_item_array(device, 0, k056832->dy);
	state_save_register_device_item_array(device, 0, k056832->layer_tile_mode);

	state_save_register_device_item(device, 0, k056832->default_layer_association);
	state_save_register_device_item(device, 0, k056832->active_layer);
	state_save_register_device_item(device, 0, k056832->linemap_enabled);
	state_save_register_device_item(device, 0, k056832->use_ext_linescroll);
	state_save_register_device_item(device, 0, k056832->uses_tile_banks);
	state_save_register_device_item(device, 0, k056832->cur_tile_bank);
	state_save_register_device_item(device, 0, k056832->rom_half);
	state_save_register_device_item_array(device, 0, k056832->all_lines_dirty);
	state_save_register_device_item_array(device, 0, k056832->page_tile_mode);

	for (i = 0; i < 8; i++)
	{
		state_save_register_device_item_array(device, i, k056832->layer_offs[i]);
		state_save_register_device_item_array(device, i, k056832->lsram_page[i]);
	}

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		state_save_register_device_item_array(device, i, k056832->line_dirty[i]);
		state_save_register_device_item(device, i, k056832->all_lines_dirty[i]);
		state_save_register_device_item(device, i, k056832->page_tile_mode[i]);
		state_save_register_device_item(device, i, k056832->last_colorbase[i]);
	}

	state_save_register_postload(device->machine, k056832_postload, k056832);
}

/***************************************************************************/
/*                                                                         */
/*                                 055555                                  */
/*                                                                         */
/***************************************************************************/

/* K055555 5-bit-per-pixel priority encoder */
/* This device has 48 8-bit-wide registers */

typedef struct _k055555_state k055555_state;
struct _k055555_state
{
	UINT8    regs[128];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k055555_state *k055555_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K055555);

	return (k055555_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k055555_write_reg( const device_config *device, UINT8 regnum, UINT8 regdat )
{
	k055555_state *k055555 = k055555_get_safe_token(device);

	static const char *const rnames[46] =
	{
		"BGC CBLK", "BGC SET", "COLSET0", "COLSET1", "COLSET2", "COLSET3", "COLCHG ON",
		"A PRI 0", "A PRI 1", "A COLPRI", "B PRI 0", "B PRI 1", "B COLPRI", "C PRI", "D PRI",
		"OBJ PRI", "SUB1 PRI", "SUB2 PRI", "SUB3 PRI", "OBJ INPRI ON", "S1 INPRI ON", "S2 INPRI ON",
		"S3 INPRI ON", "A PAL", "B PAL", "C PAL", "D PAL", "OBJ PAL", "SUB1 PAL", "SUB2 PAL", "SUB3 PAL",
		"SUB2 PAL ON", "SUB3 PAL ON", "V INMIX", "V INMIX ON", "OS INMIX", "OS INMIX ON", "SHD PRI 1",
		"SHD PRI 2", "SHD PRI 3", "SHD ON", "SHD PRI SEL", "V BRI", "OS INBRI", "OS INBRI ON", "ENABLE"
	};

	if (regdat != k055555->regs[regnum])
	{
		LOG(("5^5: %x to reg %x (%s)\n", regdat, regnum, rnames[regnum]));
	}

	k055555->regs[regnum] = regdat;
}

WRITE32_DEVICE_HANDLER( k055555_long_w )
{
	UINT8 regnum, regdat;

	if (ACCESSING_BITS_24_31)
	{
		regnum = offset << 1;
		regdat = data >> 24;
	}
	else
	{
		if (ACCESSING_BITS_8_15)
		{
			regnum = (offset << 1) + 1;
			regdat = data >> 8;
		}
		else
		{
			// logerror("5^5: unknown mem_mask %08x\n", mem_mask);
			return;
		}
	}

	k055555_write_reg(device, regnum, regdat);
}

WRITE16_DEVICE_HANDLER( k055555_word_w )
{
	if (mem_mask == 0x00ff)
	{
		k055555_write_reg(device, offset, data & 0xff);
	}
	else
	{
		k055555_write_reg(device, offset, data >> 8);
	}
}

int k055555_read_register( const device_config *device, int regnum )
{
	k055555_state *k055555 = k055555_get_safe_token(device);
	return k055555->regs[regnum];
}

int k055555_get_palette_index( const device_config *device, int idx )
{
	k055555_state *k055555 = k055555_get_safe_token(device);
	return k055555->regs[K55_PALBASE_A + idx];
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k055555 )
{
	k055555_state *k055555 = k055555_get_safe_token(device);
	state_save_register_device_item_array(device, 0, k055555->regs);
}

static DEVICE_RESET( k055555 )
{
	k055555_state *k055555 = k055555_get_safe_token(device);
	memset(k055555->regs, 0, 64 * sizeof(UINT8));
}


/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

// k054338 alpha blend / final mixer (normally used with the 55555)
// because the implementation is video dependant, this is just a
// register-handling shell.


typedef struct _k054338_state k054338_state;
struct _k054338_state
{
	UINT16    regs[32];
	int       shd_rgb[9];
	int       alphainverted;

	const device_config *screen;
	const device_config *k055555;	/* used to fill BG color */
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k054338_state *k054338_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K054338);

	return (k054338_state *)device->token;
}

INLINE const k054338_interface *k054338_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert(device->type == K054338);
	return (const k054338_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE16_DEVICE_HANDLER( k054338_word_w )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	COMBINE_DATA(k054338->regs + offset);
}

WRITE32_DEVICE_HANDLER( k054338_long_w )
{
	offset <<= 1;
	k054338_word_w(device, offset, data >> 16, mem_mask >> 16);
	k054338_word_w(device, offset + 1, data, mem_mask);
}

// returns a 16-bit '338 register
int  k054338_register_r( const device_config *device, int reg )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	return k054338->regs[reg];
}

void k054338_update_all_shadows( const device_config *device, int rushingheroes_hack )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	running_machine *machine = device->machine;
	int i, d;
	int noclip = k054338->regs[K338_REG_CONTROL] & K338_CTL_CLIPSL;

	for (i = 0; i < 9; i++)
	{
		d = k054338->regs[K338_REG_SHAD1R + i] & 0x1ff;
		if (d >= 0x100)
			d -= 0x200;
		k054338->shd_rgb[i] = d;
	}

	if (!rushingheroes_hack)
	{
		palette_set_shadow_dRGB32(machine, 0, k054338->shd_rgb[0], k054338->shd_rgb[1], k054338->shd_rgb[2], noclip);
		palette_set_shadow_dRGB32(machine, 1, k054338->shd_rgb[3], k054338->shd_rgb[4], k054338->shd_rgb[5], noclip);
		palette_set_shadow_dRGB32(machine, 2, k054338->shd_rgb[6], k054338->shd_rgb[7], k054338->shd_rgb[8], noclip);
	}
	else // rushing heroes seems to specify shadows in another format, or it's not being interpreted properly.
	{
		palette_set_shadow_dRGB32(machine, 0, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 1, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 2, -80, -80, -80, 0);
	}
}

// k054338 BG color fill
void k054338_fill_solid_bg( const device_config *device, bitmap_t *bitmap )
{
	UINT32 bgcolor;
	UINT32 *pLine;
	int x, y;

	bgcolor = (k054338_register_r(device, K338_REG_BGC_R) & 0xff) << 16;
	bgcolor |= k054338_register_r(device, K338_REG_BGC_GB);

	/* and fill the screen with it */
	for (y = 0; y < bitmap->height; y++)
	{
		pLine = (UINT32 *)bitmap->base;
		pLine += (bitmap->rowpixels * y);
		for (x = 0; x < bitmap->width; x++)
			*pLine++ = bgcolor;
	}
}

// Unified k054338/K055555 BG color fill
void k054338_fill_backcolor( const device_config *device, bitmap_t *bitmap, int mode ) // (see p.67)
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	int clipx, clipy, clipw, cliph, i, dst_pitch;
	int BGC_CBLK, BGC_SET;
	UINT32 *dst_ptr, *pal_ptr;
	int bgcolor;
	const rectangle *visarea = video_screen_get_visible_area(k054338->screen);

	clipx = visarea->min_x & ~3;
	clipy = visarea->min_y;
	clipw = (visarea->max_x - clipx + 4) & ~3;
	cliph = visarea->max_y - clipy + 1;

	dst_ptr = BITMAP_ADDR32(bitmap, clipy, 0);
	dst_pitch = bitmap->rowpixels;
	dst_ptr += clipx;

	BGC_SET = 0;
	pal_ptr = device->machine->generic.paletteram.u32;

	if (!mode || k054338->k055555 == NULL)
	{
		// single color output from CLTC
		bgcolor = (int)(k054338->regs[K338_REG_BGC_R] & 0xff) << 16 | (int)k054338->regs[K338_REG_BGC_GB];
	}
	else
	{
		BGC_CBLK = k055555_read_register(k054338->k055555, 0);
		BGC_SET  = k055555_read_register(k054338->k055555, 1);

		pal_ptr += BGC_CBLK << 9;

		// single color output from PCU2
		if (!(BGC_SET & 2))
		{
			bgcolor = *pal_ptr;
			mode = 0;
		}
		else bgcolor = 0;
	}

	if (!mode)
	{
		// single color fill
		dst_ptr += clipw;
		i = clipw = -clipw;
		do
		{
			do
			{
				dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor;
			}
			while (i += 4);

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
				do
				{
					dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor;
				}
				while (i += 4);

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
int k054338_set_alpha_level( const device_config *device, int pblend )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	UINT16 *regs;
	int ctrl, mixpri, mixset, mixlv;

	if (pblend <= 0 || pblend > 3)
	{
		return (255);
	}

	regs   = k054338->regs;
	ctrl   = k054338->regs[K338_REG_CONTROL];
	mixpri = ctrl & K338_CTL_MIXPRI;
	mixset = regs[K338_REG_PBLEND + (pblend >> 1 & 1)] >> (~pblend << 3 & 8);
	mixlv  = mixset & 0x1f;

	if (k054338->alphainverted)
		mixlv = 0x1f - mixlv;

	if (!(mixset & 0x20))
	{
		mixlv = (mixlv << 3) | (mixlv >> 2);
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
		if (mixlv && mixlv < 0x1f)
			mixlv = 0x10;

		mixlv = (mixlv << 3) | (mixlv >> 2);

		if (VERBOSE)
			popmessage("MIXSET%1d %s addition mode: %02x", pblend, (mixpri) ? "dst" : "src", mixset & 0x1f);
	}

	return mixlv;
}

void k054338_invert_alpha( const device_config *device, int invert )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	k054338->alphainverted = invert;
}


#if 0
// FIXME
void k054338->export_config( const device_config *device, int **shd_rgb )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	*shd_rgb = k054338->shd_rgb;
}
#endif

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k054338 )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	const k054338_interface *intf = k054338_get_interface(device);

	k054338->screen = devtag_get_device(device->machine, intf->screen);
	k054338->k055555 = devtag_get_device(device->machine, intf->k055555);

	k054338->alphainverted = intf->alpha_inv;

	state_save_register_device_item_array(device, 0, k054338->regs);
	state_save_register_device_item_array(device, 0, k054338->shd_rgb);
}

static DEVICE_RESET( k054338 )
{
	k054338_state *k054338 = k054338_get_safe_token(device);

	memset(k054338->regs, 0, sizeof(UINT16)*32);
	memset(k054338->shd_rgb, 0, sizeof(int)*9);
}


/***************************************************************************/
/*                                                                         */
/*                                 053250                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k053250_state k053250_state;
struct _k053250_state
{
	UINT8       regs[8];
	UINT8       *base;
	UINT16      *ram, *rammax;
	UINT16      *buffer[2];
	UINT32      rommask;
	int         page;
	int         frame, offsx, offsy;

	const device_config *screen;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053250_state *k053250_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K053250);

	return (k053250_state *)device->token;
}

INLINE const k053250_interface *k053250_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K053250));
	return (const k053250_interface *) device->static_config;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

// The DMA process should be instantaneous but since rendering in MAME is performed at VIDEO_UPDATE()
// the k053250 memory must be buffered to maintain visual integrity.
void k053250_dma( const device_config *device, int limiter )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	int last_frame, current_frame;

	current_frame = video_screen_get_frame_number(k053250->screen);
	last_frame = k053250->frame;

	if (limiter && current_frame == last_frame)
		return; // make sure we only do DMA transfer once per frame

	k053250->frame = current_frame;
	memcpy(k053250->buffer[k053250->page], k053250->ram, 0x1000);
	k053250->page ^= 1;
}

WRITE16_DEVICE_HANDLER( k053250_w )
{
	k053250_state *k053250 = k053250_get_safe_token(device);

	if (ACCESSING_BITS_0_7)
	{
		// start LVC DMA transfer at the falling edge of control register's bit1
		if (offset == 4 && !(data & 2) && (k053250->regs[4] & 2))
			k053250_dma(device, 1);

		k053250->regs[offset] = data;
	}
}

READ16_DEVICE_HANDLER( k053250_r )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	return k053250->regs[offset];
}

WRITE16_DEVICE_HANDLER( k053250_ram_w )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	COMBINE_DATA(k053250->ram + offset);
}

READ16_DEVICE_HANDLER( k053250_ram_r )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	return k053250->ram[offset];
}

READ16_DEVICE_HANDLER( k053250_rom_r )
{
	k053250_state *k053250 = k053250_get_safe_token(device);

//  if (!(k053250->regs[5] & 1)) logerror("Back: Reading rom memory with enable=0\n");

	return *(k053250->base + 0x80000 * k053250->regs[6] + 0x800 * k053250->regs[7] + (offset >> 1));
}


// Pixel data of the k053250 is nibble packed. It's preferable to be unpacked into byte format.
static void k053250_unpack_pixels(running_machine *machine, const char *region)
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
		dst_ptr[offset * 2    ] = hi_nibble;
		dst_ptr[offset * 2 + 1] = lo_nibble;
	}
	while ((--offset) >= 0);
}


// utility function to render a clipped scanline vertically or horizontally
INLINE void k053250_pdraw_scanline32(bitmap_t *bitmap, const pen_t *palette, UINT8 *source,
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

void k053250_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect, int colorbase, int flags, int priority )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	UINT16 *line_ram;
	UINT8 *pix_base, *pix_ptr, *regs;
	const pen_t *pal_base, *pal_ptr;
	UINT32 rommask, src_clipmask, src_wrapmask, dst_wrapmask;
	int map_scrollx, map_scrolly, ctrl, orientation;
	int dst_minx, dst_maxx, dst_miny, dst_maxy;
	int linedata_offs, linedata_adv, line_pos, line_start, line_end, scroll_corr;
	int color, offset, zoom, scroll, passes, i, dst_height;

	line_ram = k053250->buffer[k053250->page];	// pointer to physical line RAM
	pix_base = k053250->base;							// pointer to source pixel ROM
	rommask  = k053250->rommask;						// source ROM limit
	regs     = k053250->regs;							// pointer to registers group

	map_scrollx = (short)(regs[0] << 8 | regs[1]);		// signed horizontal scroll value
	map_scrolly = (short)(regs[2] << 8 | regs[3]);		// signed vertical scroll value
	map_scrollx -= k053250->offsx;						// add user X offset to horizontal scroll
	map_scrolly -= k053250->offsy;						// add user Y offset to vertical scroll
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

		switch (ctrl >> 5) // the upper four bits of the control register select source and target dimensions
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
	pal_base = device->machine->pens + (colorbase << 4) % device->machine->config->total_colors;

	// walk the target bitmap within the visible area vertically or horizontally, one line at a time
	for (line_pos=line_start; line_pos <= line_end; linedata_offs += linedata_adv, line_pos++)
	{
		linedata_offs &= 0x7ff;						// line info data wraps at the four-kilobyte boundary

		color  = line_ram[linedata_offs];			// get scanline color code
		if (color == 0xffff) continue;				// reject scanline if color code equals minus one

		offset   = line_ram[linedata_offs + 1];		// get first pixel offset in ROM
		if (!(color & 0xff) && !offset) continue;	// reject scanline if both color and pixel offset are zero

		// calculate physical palette location
		// there can be thirty-two color codes and each code represents sixteen pens
		pal_ptr = pal_base + ((color & 0x1f) << 4);

		// calculate physical pixel location
		// each offset unit represents two hundred and fifty six pixels and should wrap at ROM boundary for safty
		pix_ptr = pix_base + (offset << 8) % rommask;

		// get scanline zoom factor
		// For example, 0x20 doubles the length, 0x40 maintains a one-to-one length,
		// and 0x80 halves the length. The zoom center is at the beginning of the
		// scanline therefore it is not necessary to adjust render start position
		zoom    = line_ram[linedata_offs + 2];

		scroll  = (short)line_ram[linedata_offs + 3];	// get signed local scroll value for the current scanline

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
			k053250_pdraw_scanline32(bitmap, pal_ptr, pix_ptr, cliprect,
				line_pos, scroll, zoom, src_clipmask, src_wrapmask, orientation, device->machine->priority_bitmap, (UINT8)priority);

			// shift scanline position one virtual screen upward to render the wrapped end if necessary
			scroll -= dst_height;
		}
		while (--i);
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053250 )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	const k053250_interface *intf = k053250_get_interface(device);

	k053250->base = memory_region(device->machine, intf->gfx_memory_region);
	k053250->rommask = memory_region_length(device->machine, intf->gfx_memory_region);

	k053250->screen = devtag_get_device(device->machine, intf->screen);

	k053250->ram = auto_alloc_array(device->machine, UINT16, 0x6000 / 2);

	k053250->rammax = k053250->ram + 0x800;
	k053250->buffer[0] = k053250->ram + 0x2000;
	k053250->buffer[1] = k053250->ram + 0x2800;

	k053250->offsx = intf->xoff;
	k053250->offsy = intf->yoff;

	/* unpack graphics */
	k053250_unpack_pixels(device->machine, intf->gfx_memory_region);

	state_save_register_device_item_pointer(device, 0, k053250->ram, 0x6000 / 2);
	state_save_register_device_item_array(device, 0, k053250->regs);
	state_save_register_device_item(device, 0, k053250->page);
	state_save_register_device_item(device, 0, k053250->frame);
}

static DEVICE_RESET( k053250 )
{
	k053250_state *k053250 = k053250_get_safe_token(device);
	int i;

	k053250->page = 0;
	k053250->frame = -1;

	for (i = 0; i < 8; i++)
		k053250->regs[i] = 0;

}


/***************************************************************************/
/*                                                                         */
/*                                 053252                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _k053252_state k053252_state;
struct _k053252_state
{
	UINT16   regs[16];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053252_state *k053252_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K053252);

	return (k053252_state *)device->token;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ16_DEVICE_HANDLER( k053252_word_r )
{
	k053252_state *k053252 = k053252_get_safe_token(device);
	return k053252->regs[offset];
}

WRITE16_DEVICE_HANDLER( k053252_word_w )
{
	k053252_state *k053252 = k053252_get_safe_token(device);
	COMBINE_DATA(k053252->regs + offset);
}

WRITE32_DEVICE_HANDLER( k053252_long_w )
{
	offset <<= 1;
	k053252_word_w(device, offset, data >> 16, mem_mask >> 16);
	k053252_word_w(device, offset + 1, data, mem_mask);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053252 )
{
	k053252_state *k053252 = k053252_get_safe_token(device);

	state_save_register_device_item_array(device, 0, k053252->regs);
}

static DEVICE_RESET( k053252 )
{
	k053252_state *k053252 = k053252_get_safe_token(device);
	int i;

	for (i = 0; i < 16; i++)
		k053252->regs[i] = 0;
}


// Newer Konami devices

// from video/gticlub.c



/*****************************************************************************/
/* Konami K001006 Custom 3D Texel Renderer chip (KS10081) */

/***************************************************************************/
/*                                                                         */
/*                                  001006                                 */
/*                                                                         */
/***************************************************************************/

typedef struct _k001006_state k001006_state;
struct _k001006_state
{
	const device_config *screen;

	UINT16 *     pal_ram;
	UINT16 *     unknown_ram;
	UINT32       addr;
	int          device_sel;

	UINT32 *     palette;

	const char     *gfx_region;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001006_state *k001006_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K001006);

	return (k001006_state *)device->token;
}

INLINE const k001006_interface *k001006_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K001006));
	return (const k001006_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ32_DEVICE_HANDLER( k001006_r )
{
	k001006_state *k001006 = k001006_get_safe_token(device);

	if (offset == 1)
	{
		switch (k001006->device_sel)
		{
			case 0x0b:		// CG Board ROM read
			{
				UINT16 *rom = (UINT16*)memory_region(device->machine, k001006->gfx_region);
				return rom[k001006->addr / 2] << 16;
			}
			case 0x0d:		// Palette RAM read
			{
				UINT32 addr = k001006->addr;

				k001006->addr += 2;
				return k001006->pal_ram[addr >> 1];
			}
			case 0x0f:		// Unknown RAM read
			{
				return k001006->unknown_ram[k001006->addr++];
			}
			default:
			{
				fatalerror("k001006_r, unknown device %02X", k001006->device_sel);
			}
		}
	}
	return 0;
}

WRITE32_DEVICE_HANDLER( k001006_w )
{
	k001006_state *k001006 = k001006_get_safe_token(device);

	if (offset == 0)
	{
		COMBINE_DATA(&k001006->addr);
	}
	else if (offset == 1)
	{
		switch (k001006->device_sel)
		{
			case 0xd:	// Palette RAM write
			{
				int r, g, b, a;
				UINT32 index = k001006->addr;

				k001006->pal_ram[index >> 1] = data & 0xffff;

				a = (data & 0x8000) ? 0x00 : 0xff;
				b = ((data >> 10) & 0x1f) << 3;
				g = ((data >>  5) & 0x1f) << 3;
				r = ((data >>  0) & 0x1f) << 3;
				b |= (b >> 5);
				g |= (g >> 5);
				r |= (r >> 5);
				k001006->palette[index >> 1] = MAKE_ARGB(a, r, g, b);

				k001006->addr += 2;
				break;
			}
			case 0xf:	// Unknown RAM write
			{
			//  mame_printf_debug("Unknown RAM %08X = %04X\n", k001006->addr, data & 0xffff);
				k001006->unknown_ram[k001006->addr++] = data & 0xffff;
				break;
			}
			default:
			{
				mame_printf_debug("k001006_w: device %02X, write %04X to %08X\n", k001006->device_sel, data & 0xffff, k001006->addr++);
			}
		}
	}
	else if (offset == 2)
	{
		if (ACCESSING_BITS_16_31)
		{
			k001006->device_sel = (data >> 16) & 0xf;
		}
	}
}

UINT32 k001006_get_palette( const device_config *device, int index )
{
	k001006_state *k001006 = k001006_get_safe_token(device);
	return k001006->palette[index];
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k001006 )
{
	k001006_state *k001006 = k001006_get_safe_token(device);
	const k001006_interface *intf = k001006_get_interface(device);

	k001006->pal_ram = auto_alloc_array_clear(device->machine, UINT16, 0x800);
	k001006->unknown_ram = auto_alloc_array_clear(device->machine, UINT16, 0x1000);
	k001006->palette = auto_alloc_array(device->machine, UINT32, 0x800);

	k001006->gfx_region = intf->gfx_region;

	state_save_register_device_item_pointer(device, 0, k001006->pal_ram, 0x800*sizeof(UINT16));
	state_save_register_device_item_pointer(device, 0, k001006->unknown_ram, 0x1000*sizeof(UINT16));
	state_save_register_device_item_pointer(device, 0, k001006->palette, 0x800*sizeof(UINT32));
	state_save_register_device_item(device, 0, k001006->device_sel);
	state_save_register_device_item(device, 0, k001006->addr);
}

static DEVICE_RESET( k001006 )
{
	k001006_state *k001006 = k001006_get_safe_token(device);

	k001006->addr = 0;
	k001006->device_sel = 0;
	memset(k001006->palette, 0, 0x800*sizeof(UINT32));
}


/*****************************************************************************/
/* Konami K001005 Custom 3D Pixel Renderer chip (KS10071) */

/***************************************************************************/
/*                                                                         */
/*                                  001005                                 */
/*                                                                         */
/***************************************************************************/

#include "video/poly.h"
#include "cpu/sharc/sharc.h"

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
};

typedef struct _k001005_state k001005_state;
struct _k001005_state
{
	const device_config *screen;
	const device_config *cpu;
	const device_config *dsp;
	const device_config *k001006_1;
	const device_config *k001006_2;

	UINT8  *     texture;
	UINT16 *     ram[2];
	UINT32 *     fifo;
	UINT32 *     _3d_fifo;

	UINT32    status;
	bitmap_t *bitmap[2];
	bitmap_t *zbuffer;
	rectangle cliprect;
	int    ram_ptr;
	int    fifo_read_ptr;
	int    fifo_write_ptr;
	int    _3d_fifo_ptr;

	int tex_mirror_table[4][128];

	int bitmap_page;

	poly_manager *poly;
	poly_vertex prev_v[4];
	int prev_poly_type;

	UINT8 *gfxrom;
};

static const int decode_x_gti[8] = {  0, 16, 2, 18, 4, 20, 6, 22 };
static const int decode_y_gti[16] = {  0, 8, 32, 40, 1, 9, 33, 41, 64, 72, 96, 104, 65, 73, 97, 105 };

static const int decode_x_zr107[8] = {  0, 16, 1, 17, 2, 18, 3, 19 };
static const int decode_y_zr107[16] = {  0, 8, 32, 40, 4, 12, 36, 44, 64, 72, 96, 104, 68, 76, 100, 108 };


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001005_state *k001005_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K001005);

	return (k001005_state *)device->token;
}

INLINE const k001005_interface *k001005_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K001005));
	return (const k001005_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static void k001005_render_polygons( const device_config *device );

// rearranges the texture data to a more practical order
void k001005_preprocess_texture_data( UINT8 *rom, int length, int gticlub )
{
	int index;
	int i, x, y;
	UINT8 temp[0x40000];

	const int *decode_x;
	const int *decode_y;

	if (gticlub)
	{
		decode_x = decode_x_gti;
		decode_y = decode_y_gti;
	}
	else
	{
		decode_x = decode_x_zr107;
		decode_y = decode_y_zr107;
	}

	for (index = 0; index < length; index += 0x40000)
	{
		int offset = index;

		memset(temp, 0, 0x40000);

		for (i = 0; i < 0x800; i++)
		{
			int tx = ((i & 0x400) >> 5) | ((i & 0x100) >> 4) | ((i & 0x40) >> 3) | ((i & 0x10) >> 2) | ((i & 0x4) >> 1) | (i & 0x1);
			int ty = ((i & 0x200) >> 5) | ((i & 0x80) >> 4) | ((i & 0x20) >> 3) | ((i & 0x8) >> 2) | ((i & 0x2) >> 1);

			tx <<= 3;
			ty <<= 4;

			for (y = 0; y < 16; y++)
			{
				for (x = 0; x < 8; x++)
				{
					UINT8 pixel = rom[offset + decode_y[y] + decode_x[x]];

					temp[((ty + y) * 512) + (tx + x)] = pixel;
				}
			}

			offset += 128;
		}

		memcpy(&rom[index], temp, 0x40000);
	}
}

void k001005_swap_buffers( const device_config *device )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	k001005->bitmap_page ^= 1;

	//if (k001005->status == 2)
	{
		bitmap_fill(k001005->bitmap[k001005->bitmap_page], &k001005->cliprect, device->machine->pens[0] & 0x00ffffff);
		bitmap_fill(k001005->zbuffer, &k001005->cliprect, 0xffffffff);
	}
}

READ32_DEVICE_HANDLER( k001005_r )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	switch(offset)
	{
		case 0x000:			// FIFO read, high 16 bits
		{
			UINT16 value = k001005->fifo[k001005->fifo_read_ptr] >> 16;
		//  mame_printf_debug("FIFO_r0: %08X\n", k001005->fifo_ptr);
			return value;
		}

		case 0x001:			// FIFO read, low 16 bits
		{
			UINT16 value = k001005->fifo[k001005->fifo_read_ptr] & 0xffff;
		//  mame_printf_debug("FIFO_r1: %08X\n", k001005->fifo_ptr);

			if (k001005->status != 1 && k001005->status != 2)
			{
				if (k001005->fifo_read_ptr < 0x3ff)
				{
					//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(k001005->dsp, 1, CLEAR_LINE);
				}
				else
				{
					//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
				}
			}
			else
			{
				//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
			}

			k001005->fifo_read_ptr++;
			k001005->fifo_read_ptr &= 0x7ff;
			return value;
		}

		case 0x11b:			// status ?
			return 0x8002;

		case 0x11c:			// slave status ?
			return 0x8000;

		case 0x11f:
			if (k001005->ram_ptr >= 0x400000)
			{
				return k001005->ram[1][(k001005->ram_ptr++) & 0x3fffff];
			}
			else
			{
				return k001005->ram[0][(k001005->ram_ptr++) & 0x3fffff];
			}

		default:
			//mame_printf_debug("k001005->r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
	return 0;
}

WRITE32_DEVICE_HANDLER( k001005_w )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	switch (offset)
	{
		case 0x000:			// FIFO write
		{
			if (k001005->status != 1 && k001005->status != 2)
			{
				if (k001005->fifo_write_ptr < 0x400)
				{
					//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
				}
				else
				{
					//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(k001005->dsp, 1, CLEAR_LINE);
				}
			}
			else
			{
				//cpu_set_input_line(k001005->dsp, SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
			}

	    //  mame_printf_debug("K001005 FIFO write: %08X at %08X\n", data, cpu_get_pc(space->cpu));
			k001005->fifo[k001005->fifo_write_ptr] = data;
			k001005->fifo_write_ptr++;
			k001005->fifo_write_ptr &= 0x7ff;

			k001005->_3d_fifo[k001005->_3d_fifo_ptr++] = data;

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (cpu_get_pc(k001005->cpu) == 0x201ee)
			{
				// This is used to make the SHARC timeout
				cpu_spinuntil_trigger(k001005->cpu, 10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (cpu_get_pc(k001005->cpu) == 0x201e6)
			{
				// This is used to make the SHARC timeout
				cpu_spinuntil_trigger(k001005->cpu, 10000);
			}

			break;
		}

		case 0x100: break;

	//  case 0x10a:     poly_r = data & 0xff; break;
	//  case 0x10b:     poly_g = data & 0xff; break;
	//  case 0x10c:     poly_b = data & 0xff; break;

		case 0x11a:
			k001005->status = data;
			k001005->fifo_write_ptr = 0;
			k001005->fifo_read_ptr = 0;

			if (data == 2 && k001005->_3d_fifo_ptr > 0)
			{
				k001005_swap_buffers(device);
				k001005_render_polygons(device);
				poly_wait(k001005->poly, "render_polygons");
				k001005->_3d_fifo_ptr = 0;
			}
			break;

		case 0x11d:
			k001005->fifo_write_ptr = 0;
			k001005->fifo_read_ptr = 0;
			break;

		case 0x11e:
			k001005->ram_ptr = data;
			break;

		case 0x11f:
			if (k001005->ram_ptr >= 0x400000)
			{
				k001005->ram[1][(k001005->ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			else
			{
				k001005->ram[0][(k001005->ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			break;

		default:
			//mame_printf_debug("k001005->w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}

}

/* emu/video/poly.c cannot handle atm callbacks passing a device parameter */
#define POLY_DEVICE 0

#if POLY_DEVICE
static void draw_scanline( const device_config *device, void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	float z = extent->param[0].start;
	float dz = extent->param[0].dpdx;
	UINT32 *fb = BITMAP_ADDR32(destmap, scanline, 0);
	UINT32 *zb = BITMAP_ADDR32(k001005->zbuffer, scanline, 0);
	UINT32 color = extra->color;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;

		if (iz <= zb[x])
		{
			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		z += dz;
	}
}
#endif

#if POLY_DEVICE
static void draw_scanline_tex( const device_config *device, void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	UINT8 *texrom = k001005->gfxrom + (extra->texture_page * 0x40000);
	const device_config *pal_device = (extra->texture_palette & 0x8) ? k001005->k001006_2 : k001005->k001006_1;
	int palette_index = (extra->texture_palette & 0x7) * 256;
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float w = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float dw = extent->param[3].dpdx;
	int texture_mirror_x = extra->texture_mirror_x;
	int texture_mirror_y = extra->texture_mirror_y;
	int texture_x = extra->texture_x;
	int texture_y = extra->texture_y;
	int x;

	UINT32 *fb = BITMAP_ADDR32(destmap, scanline, 0);
	UINT32 *zb = BITMAP_ADDR32(k001005->zbuffer, scanline, 0);

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;
		//int iu = u >> 16;
		//int iv = v >> 16;

		if (iz < zb[x])
		{
			float oow = 1.0f / w;
			UINT32 color;
			int iu, iv;
			int iiv, iiu, texel;

			iu = u * oow;
			iv = v * oow;

			iiu = texture_x + k001005->tex_mirror_table[texture_mirror_x][(iu >> 4) & 0x7f];
			iiv = texture_y + k001005->tex_mirror_table[texture_mirror_y][(iv >> 4) & 0x7f];
			texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
			color = k001006_get_palette(pal_device, palette_index + texel);

			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		u += du;
		v += dv;
		z += dz;
		w += dw;
	}
}
#endif


static void k001005_render_polygons( const device_config *device )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	int i, j;
#if POLY_DEVICE
	const rectangle *visarea = video_screen_get_visible_area(k001005->screen);
#endif

//  mame_printf_debug("k001005->fifo_ptr = %08X\n", k001005->_3d_fifo_ptr);

	for (i = 0; i < k001005->_3d_fifo_ptr; i++)
	{
		if (k001005->_3d_fifo[i] == 0x80000003)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int index = i;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = 0;	/* ??? */
			}

			++index;

			r = (k001005->_3d_fifo[index] >>  0) & 0xff;
			g = (k001005->_3d_fifo[index] >>  8) & 0xff;
			b = (k001005->_3d_fifo[index] >> 16) & 0xff;
			a = (k001005->_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			++index;

			extra->color = color;
#if POLY_DEVICE
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//          poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page],  visarea, draw_scanline, 1, 4, v);
#endif
			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x800000ae || k001005->_3d_fifo[i] == 0x8000008e ||
				 k001005->_3d_fifo[i] == 0x80000096 || k001005->_3d_fifo[i] == 0x800000b6 ||
				 k001005->_3d_fifo[i] == 0x8000002e || k001005->_3d_fifo[i] == 0x8000000e ||
				 k001005->_3d_fifo[i] == 0x80000016 || k001005->_3d_fifo[i] == 0x80000036 ||
				 k001005->_3d_fifo[i] == 0x800000aa || k001005->_3d_fifo[i] == 0x800000a8 ||
				 k001005->_3d_fifo[i] == 0x800000b2)
		{
			// 0x00: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx    Command
			//
			// 0x01: xxxx---- -------- -------- --------    Texture palette
			// 0x01: -------- -------x xxxx---- --------    Texture page
			// 0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
			// 0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
			poly_vertex v[4];
			int tx, ty;
			UINT32 color = 0;
			UINT32 header;
			UINT32 command;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			command = k001005->_3d_fifo[index++];
			header = k001005->_3d_fifo[index++];

			for (j = 0; j < 4; j++)
			{
				INT16 u2, v2;
				int x, y, z;
				int end = 0;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = k001005->_3d_fifo[index] & 0x4000;
				end = k001005->_3d_fifo[index] & 0x8000;
				++index;

				z = k001005->_3d_fifo[index];
				++index;

				if (end)
				{
					color = k001005->_3d_fifo[index];
					++index;

					u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
					v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}
				else
				{
					u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
					v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);
				v[j].p[3] = 1.0f / v[j].p[0];
				v[j].p[1] = u2 * v[j].p[3];
				v[j].p[2] = v2 * v[j].p[3];

				++num_verts;

				if (end)
					break;
			}

			ty = ((header & 0x400) >> 5) |
				 ((header & 0x100) >> 4) |
				 ((header & 0x040) >> 3) |
				 ((header & 0x010) >> 2) |
				 ((header & 0x004) >> 1) |
				 ((header & 0x001) >> 0);

			tx = ((header & 0x800) >> 6) |
				 ((header & 0x200) >> 5) |
				 ((header & 0x080) >> 4) |
				 ((header & 0x020) >> 3) |
				 ((header & 0x008) >> 2) |
				 ((header & 0x002) >> 1);

			extra->texture_x = tx * 8;
			extra->texture_y = ty * 8;

			extra->texture_page = (header >> 12) & 0x1f;
			extra->texture_palette = (header >> 28) & 0xf;

			extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
			extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

			extra->color = color;

			if (num_verts < 3)
			{
#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &v[0], &v[1]);
				if (k001005->prev_poly_type)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &k001005->prev_v[3], &v[0]);
//              if (k001005->prev_poly_type)
//                  poly_render_quad(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &k001005->prev_v[3], &v[0], &v[1]);
//              else
//                  poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &v[0], &v[1]);
#endif
				memcpy(&k001005->prev_v[0], &k001005->prev_v[2], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[2], &v[0], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[3], &v[1], sizeof(poly_vertex));
			}
			else
			{
#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (num_verts > 3)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, num_verts, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			}

			k001005->prev_poly_type = poly_type;

			while ((k001005->_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < k001005->_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &k001005->prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &k001005->prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					INT16 u2, v2;
					int x, y, z;
					int end = 0;

					x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
					y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = k001005->_3d_fifo[index] & 0x4000;
					end = k001005->_3d_fifo[index] & 0x8000;
					++index;

					z = k001005->_3d_fifo[index];
					++index;

					if (end)
					{
						color = k001005->_3d_fifo[index];
						++index;

						u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
						v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}
					else
					{
						u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
						v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);
					v[j].p[3] = 1.0f / v[j].p[0];
					v[j].p[1] = u2 * v[j].p[3];
					v[j].p[2] = v2 * v[j].p[3];

					++new_verts;

					if (end)
						break;
				}

				extra->texture_x = tx * 8;
				extra->texture_y = ty * 8;

				extra->texture_page = (header >> 12) & 0x1f;
				extra->texture_palette = (header >> 28) & 0xf;

				extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
				extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline_tex, 4, new_verts + 2, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x80000006 || k001005->_3d_fifo[i] == 0x80000026 ||
				 k001005->_3d_fifo[i] == 0x80000020 || k001005->_3d_fifo[i] == 0x80000022)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y, z;
				int end = 0;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = k001005->_3d_fifo[index] & 0x4000;
				end = k001005->_3d_fifo[index] & 0x8000;
				++index;

				z = k001005->_3d_fifo[index];
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);

				++num_verts;

				if (end)
					break;
			}

			r = (k001005->_3d_fifo[index] >>  0) & 0xff;
			g = (k001005->_3d_fifo[index] >>  8) & 0xff;
			b = (k001005->_3d_fifo[index] >> 16) & 0xff;
			a = (k001005->_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra->color = color;

#if POLY_DEVICE
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			if (num_verts > 3)
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[2], &v[3], &v[0]);
//          poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, num_verts, v);
#endif
			memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);

			while ((k001005->_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < k001005->_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &k001005->prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &k001005->prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					int x, y, z;
					int end = 0;

					x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
					y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = k001005->_3d_fifo[index] & 0x4000;
					end = k001005->_3d_fifo[index] & 0x8000;
					++index;

					z = k001005->_3d_fifo[index];
					++index;

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);

					++new_verts;

					if (end)
						break;
				}

				r = (k001005->_3d_fifo[index] >>  0) & 0xff;
				g = (k001005->_3d_fifo[index] >>  8) & 0xff;
				b = (k001005->_3d_fifo[index] >> 16) & 0xff;
				a = (k001005->_3d_fifo[index] >> 24) & 0xff;
				color = (a << 24) | (r << 16) | (g << 8) | (b);
				index++;

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], visarea, draw_scanline, 1, new_verts + 2, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x80000000)
		{

		}
		else if ((k001005->_3d_fifo[i] & 0xffffff00) == 0x80000000)
		{
			/*
            mame_printf_debug("Unknown polygon type %08X:\n", k001005->_3d_fifo[i]);
            for (j = 0; j < 0x20; j++)
            {
                mame_printf_debug("  %02X: %08X\n", j, k001005->_3d_fifo[i + 1 + j]);
            }
            mame_printf_debug("\n");
            */
		}
	}
}

void k001005_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	int i, j;

	memcpy(&k001005->cliprect, cliprect, sizeof(rectangle));

	for (j = cliprect->min_y; j <= cliprect->max_y; j++)
	{
		UINT32 *bmp = BITMAP_ADDR32(bitmap, j, 0);
		UINT32 *src = BITMAP_ADDR32(k001005->bitmap[k001005->bitmap_page ^ 1], j, 0);

		for (i = cliprect->min_x; i <= cliprect->max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const k001005_interface *intf = k001005_get_interface(device);
	int i, width, height;

	k001005->cpu = devtag_get_device(device->machine, intf->cpu);
	k001005->dsp = devtag_get_device(device->machine, intf->dsp);
	k001005->k001006_1 = devtag_get_device(device->machine, intf->k001006_1);
	k001005->k001006_2 = devtag_get_device(device->machine, intf->k001006_2);

	k001005->screen = devtag_get_device(device->machine, intf->screen);
	width = video_screen_get_width(k001005->screen);
	height = video_screen_get_height(k001005->screen);
	k001005->zbuffer = auto_bitmap_alloc(device->machine, width, height, BITMAP_FORMAT_INDEXED32);

	k001005->gfxrom = memory_region(device->machine, intf->gfx_memory_region);

	k001005->bitmap[0] = video_screen_auto_bitmap_alloc(k001005->screen);
	k001005->bitmap[1] = video_screen_auto_bitmap_alloc(k001005->screen);

	k001005->texture = auto_alloc_array(device->machine, UINT8, 0x800000);

	k001005->ram[0] = auto_alloc_array(device->machine, UINT16, 0x140000);
	k001005->ram[1] = auto_alloc_array(device->machine, UINT16, 0x140000);

	k001005->fifo = auto_alloc_array(device->machine, UINT32, 0x800);

	k001005->_3d_fifo = auto_alloc_array(device->machine, UINT32, 0x10000);

	k001005->poly = poly_alloc(device->machine, 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	for (i = 0; i < 128; i++)
	{
		k001005->tex_mirror_table[0][i] = i & 0x3f;
		k001005->tex_mirror_table[1][i] = i & 0x3f;
		k001005->tex_mirror_table[2][i] = ((i & 0x3f) >= 0x20) ? (0x1f - (i & 0x1f)) : i & 0x1f;
		k001005->tex_mirror_table[3][i] = ((i & 0x7f) >= 0x40) ? (0x3f - (i & 0x3f)) : i & 0x3f;
	}


	state_save_register_device_item_pointer(device, 0, k001005->texture, 0x800000);
	state_save_register_device_item_pointer(device, 0, k001005->ram[0], 0x140000);
	state_save_register_device_item_pointer(device, 0, k001005->ram[1], 0x140000);
	state_save_register_device_item_pointer(device, 0, k001005->fifo, 0x800);
	state_save_register_device_item_pointer(device, 0, k001005->_3d_fifo, 0x10000);
	state_save_register_device_item(device, 0, k001005->status);
	state_save_register_device_item(device, 0, k001005->ram_ptr);
	state_save_register_device_item(device, 0, k001005->fifo_read_ptr);
	state_save_register_device_item(device, 0, k001005->fifo_write_ptr);
	state_save_register_device_item(device, 0, k001005->_3d_fifo_ptr);
	state_save_register_device_item(device, 0, k001005->bitmap_page);
	state_save_register_device_item(device, 0, k001005->prev_poly_type);
	state_save_register_device_item_bitmap(device, 0, k001005->bitmap[0]);
	state_save_register_device_item_bitmap(device, 0, k001005->bitmap[1]);
	state_save_register_device_item_bitmap(device, 0, k001005->zbuffer);

	// FIXME: shall we save poly as well?
}

static DEVICE_RESET( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	k001005->status = 0;
	k001005->ram_ptr = 0;
	k001005->fifo_read_ptr = 0;
	k001005->fifo_write_ptr = 0;
	k001005->_3d_fifo_ptr = 0;
	k001005->bitmap_page = 0;

	memset(k001005->prev_v, 0, sizeof(k001005->prev_v));
	k001005->prev_poly_type = 0;
}

static DEVICE_STOP( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	poly_free(k001005->poly);
}


// from drivers/nwk-tr

/***************************************************************************/
/*                                                                         */
/*                                  001604                                 */
/*                                                                         */
/***************************************************************************/


typedef struct _k001604_state k001604_state;
struct _k001604_state
{
	const device_config *screen;
	tilemap_t        *layer_8x8[2];
	tilemap_t        *layer_roz[2];
	int            gfx_index[2];

	UINT32 *       tile_ram;
	UINT32 *       char_ram;
	UINT32 *       reg;

	int            layer_size;
	int            roz_size;
};


#define K001604_NUM_TILES_LAYER0		16384
#define K001604_NUM_TILES_LAYER1		4096

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001604_state *k001604_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K001604);

	return (k001604_state *)device->token;
}

INLINE const k001604_interface *k001604_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K001604));
	return (const k001604_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static const gfx_layout k001604_char_layout_layer_8x8 =
{
	8, 8,
	K001604_NUM_TILES_LAYER0,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static const gfx_layout k001604_char_layout_layer_16x16 =
{
	16, 16,
	K001604_NUM_TILES_LAYER1,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16, 9*16, 8*16, 11*16, 10*16, 13*16, 12*16, 15*16, 14*16 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256 },
	16*256
};


/* FIXME: The TILEMAP_MAPPER below depends on parameters passed by the device interface (being game dependent).
we might simplify the code, by passing the whole TILEMAP_MAPPER as a callback in the interface, but is it really worth? */

static TILEMAP_MAPPER( k001604_scan_layer_8x8_0_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col;
}

static TILEMAP_MAPPER( k001604_scan_layer_8x8_0_size1 )
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col;
}

static TILEMAP_MAPPER( k001604_scan_layer_8x8_1_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64;
}

static TILEMAP_MAPPER( k001604_scan_layer_8x8_1_size1 )
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 64;
}

static TILEMAP_MAPPER( slrasslt_scan_layer_8x8_0_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 16384;
}

static TILEMAP_MAPPER( slrasslt_scan_layer_8x8_1_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64 + 16384;
}

static TILEMAP_MAPPER( k001604_scan_layer_roz_0_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col;
}

static TILEMAP_MAPPER( k001604_scan_layer_roz_0_size1 )
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 128;
}

static TILEMAP_MAPPER( k001604_scan_layer_roz_1_size0 )
{
	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64;
}

static TILEMAP_MAPPER( k001604_scan_layer_roz_1_size1 )
{
	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 128 + 64;
}

static TILE_GET_INFO_DEVICE( k001604_tile_info_layer_8x8 )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	UINT32 val = k001604->tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_DEVICE(k001604->gfx_index[0], tile, color, flags);
}

static TILE_GET_INFO_DEVICE( k001604_tile_info_layer_roz )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	UINT32 val = k001604->tile_ram[tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x7ff;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += k001604->roz_size ? 0x800 : 0x2000;

	SET_TILE_INFO_DEVICE(k001604->gfx_index[k001604->roz_size], tile, color, flags);
}


void k001604_draw_back_layer( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	int layer;
	int num_layers;
	bitmap_fill(bitmap, cliprect, 0);

	num_layers = k001604->layer_size ? 2 : 1;

	for (layer = 0; layer < num_layers; layer++)
	{
		int reg = 0x08;

		INT32 x  = (INT16)((k001604->reg[reg + 0] >> 16) & 0xffff);
		INT32 y  = (INT16)((k001604->reg[reg + 0] >>  0) & 0xffff);
		INT32 xx = (INT16)((k001604->reg[reg + 1] >>  0) & 0xffff);
		INT32 xy = (INT16)((k001604->reg[reg + 1] >> 16) & 0xffff);
		INT32 yx = (INT16)((k001604->reg[reg + 2] >>  0) & 0xffff);
		INT32 yy = (INT16)((k001604->reg[reg + 2] >> 16) & 0xffff);

		x  = (x + 320) * 256;
		y  = (y + 208) * 256;
		xx = (xx);
		xy = (-xy);
		yx = (-yx);
		yy = (yy);

		if ((k001604->reg[0x6c / 4] & (0x08 >> layer)) != 0)
		{
			tilemap_draw_roz(bitmap, cliprect, k001604->layer_roz[layer],
							 x << 5, y << 5, xx << 5, xy << 5, yx << 5, yy << 5, 1, 0, 0);
		}
	}
}

void k001604_draw_front_layer( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	//tilemap_draw(bitmap, cliprect, k001604->layer_8x8[1], 0,0);
	tilemap_draw(bitmap, cliprect, k001604->layer_8x8[0], 0,0);
}

READ32_DEVICE_HANDLER( k001604_tile_r )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	return k001604->tile_ram[offset];
}

READ32_DEVICE_HANDLER( k001604_char_r )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60 / 4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	return k001604->char_ram[addr];
}

WRITE32_DEVICE_HANDLER( k001604_tile_w )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	int x, y;
	COMBINE_DATA(k001604->tile_ram + offset);

	if (k001604->layer_size)
	{
		x = offset & 0xff;
		y = offset / 256;
	}
	else
	{
		x = offset & 0x7f;
		y = offset / 128;
	}

	if (k001604->layer_size)
	{
		if (x < 64)
		{
			tilemap_mark_tile_dirty(k001604->layer_8x8[0], offset);
		}
		else if (x < 128)
		{
			tilemap_mark_tile_dirty(k001604->layer_8x8[1], offset);
		}
		else if (x < 192)
		{
			tilemap_mark_tile_dirty(k001604->layer_roz[0], offset);
		}
		else
		{
			tilemap_mark_tile_dirty(k001604->layer_roz[1], offset);
		}
	}
	else
	{
		if (x < 64)
		{
			tilemap_mark_tile_dirty(k001604->layer_8x8[0], offset);
			tilemap_mark_tile_dirty(k001604->layer_roz[0], offset);
		}
		else
		{
			tilemap_mark_tile_dirty(k001604->layer_8x8[1], offset);
			tilemap_mark_tile_dirty(k001604->layer_roz[1], offset);
		}
	}
}

WRITE32_DEVICE_HANDLER( k001604_char_w )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	COMBINE_DATA(k001604->char_ram + addr);

	gfx_element_mark_dirty(device->machine->gfx[k001604->gfx_index[0]], addr / 32);
	gfx_element_mark_dirty(device->machine->gfx[k001604->gfx_index[1]], addr / 128);
}

WRITE32_DEVICE_HANDLER( k001604_reg_w )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	COMBINE_DATA(k001604->reg + offset);

	switch (offset)
	{
		case 0x8:
		case 0x9:
		case 0xa:
			//printf("K001604_reg_w %02X, %08X, %08X\n", offset, data, mem_mask);
			break;
	}

	if (offset != 0x08 && offset != 0x09 && offset != 0x0a /*&& offset != 0x17 && offset != 0x18*/)
	{
		//printf("K001604_reg_w (%d), %02X, %08X, %08X at %08X\n", chip, offset, data, mem_mask, cpu_get_pc(space->cpu));
	}
}

READ32_DEVICE_HANDLER( k001604_reg_r )
{
//  int chip = get_cgboard_id();
	k001604_state *k001604 = k001604_get_safe_token(device);

	switch (offset)
	{
		case 0x54/4:	return mame_rand(device->machine) << 16; break;
		case 0x5c/4:	return mame_rand(device->machine) << 16 | mame_rand(device->machine); break;
	}

	return k001604->reg[offset];
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k001604 )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	const k001604_interface *intf = k001604_get_interface(device);
	int roz_tile_size;

	k001604->layer_size = intf->layer_size;		// 0 -> width = 128 tiles, 1 -> width = 256 tiles
	k001604->roz_size = intf->roz_size;		// 0 -> 8x8, 1 -> 16x16

	k001604->gfx_index[0] = intf->gfx_index_1;
	k001604->gfx_index[1] = intf->gfx_index_2;

	k001604->char_ram = auto_alloc_array(device->machine, UINT32, 0x200000 / 4);
	k001604->tile_ram = auto_alloc_array(device->machine, UINT32, 0x20000 / 4);
	k001604->reg = auto_alloc_array(device->machine, UINT32, 0x400 / 4);

	/* create tilemaps */
	roz_tile_size = k001604->roz_size ? 16 : 8;
	if (!intf->is_slrasslt)
	{
		if (k001604->layer_size)
		{
			k001604->layer_8x8[0] = tilemap_create_device(device, k001604_tile_info_layer_8x8, k001604_scan_layer_8x8_0_size1, 8, 8, 64, 64);
			k001604->layer_8x8[1] = tilemap_create_device(device, k001604_tile_info_layer_8x8, k001604_scan_layer_8x8_1_size1, 8, 8, 64, 64);
			k001604->layer_roz[0] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_0_size1, roz_tile_size, roz_tile_size, 64, 64);
			k001604->layer_roz[1] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_1_size1, roz_tile_size, roz_tile_size, 64, 64);
		}
		else
		{
			k001604->layer_8x8[0] = tilemap_create_device(device, k001604_tile_info_layer_8x8, k001604_scan_layer_8x8_0_size0, 8, 8, 64, 64);
			k001604->layer_8x8[1] = tilemap_create_device(device, k001604_tile_info_layer_8x8, k001604_scan_layer_8x8_1_size0, 8, 8, 64, 64);
			k001604->layer_roz[0] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_0_size0, roz_tile_size, roz_tile_size, 128, 64);
			k001604->layer_roz[1] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_1_size0, roz_tile_size, roz_tile_size, 64, 64);
		}
	}
	else	/* slrasslt has shifted tilemaps (but only has k001604->layer_size =  0) */
	{
		k001604->layer_8x8[0] = tilemap_create_device(device, k001604_tile_info_layer_8x8, slrasslt_scan_layer_8x8_0_size0, 8, 8, 64, 64);
		k001604->layer_8x8[1] = tilemap_create_device(device, k001604_tile_info_layer_8x8, slrasslt_scan_layer_8x8_1_size0, 8, 8, 64, 64);
		k001604->layer_roz[0] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_0_size0, roz_tile_size, roz_tile_size, 128, 64);
		k001604->layer_roz[1] = tilemap_create_device(device, k001604_tile_info_layer_roz, k001604_scan_layer_roz_1_size0, roz_tile_size, roz_tile_size, 64, 64);
	}

	tilemap_set_transparent_pen(k001604->layer_8x8[0], 0);
	tilemap_set_transparent_pen(k001604->layer_8x8[1], 0);

	device->machine->gfx[k001604->gfx_index[0]] = gfx_element_alloc(device->machine, &k001604_char_layout_layer_8x8, (UINT8*)&k001604->char_ram[0], device->machine->config->total_colors / 16, 0);
	device->machine->gfx[k001604->gfx_index[1]] = gfx_element_alloc(device->machine, &k001604_char_layout_layer_16x16, (UINT8*)&k001604->char_ram[0], device->machine->config->total_colors / 16, 0);

	state_save_register_device_item_pointer(device, 0, k001604->reg, 0x400 / 4);
	state_save_register_device_item_pointer(device, 0, k001604->char_ram, 0x200000 / 4);
	state_save_register_device_item_pointer(device, 0, k001604->tile_ram, 0x20000 / 4);
}

static DEVICE_RESET( k001604 )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	memset(k001604->char_ram, 0, 0x200000);
	memset(k001604->tile_ram, 0, 0x10000);
	memset(k001604->reg, 0, 0x400);
}


// from drivers/hornet.c

/***************************************************************************/
/*                                                                         */
/*                                  037122                                 */
/*                                                                         */
/***************************************************************************/

typedef struct _k037122_state k037122_state;
struct _k037122_state
{
	const device_config *screen;
	tilemap_t        *layer[2];
	int            gfx_index;

	UINT32 *       tile_ram;
	UINT32 *       char_ram;
	UINT32 *       reg;
};


#define K037122_NUM_TILES		16384

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k037122_state *k037122_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K037122);

	return (k037122_state *)device->token;
}

INLINE const k037122_interface *k037122_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K037122));
	return (const k037122_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static const gfx_layout k037122_char_layout =
{
	8, 8,
	K037122_NUM_TILES,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static TILE_GET_INFO_DEVICE( k037122_tile_info_layer0 )
{
	k037122_state *k037122 = k037122_get_safe_token(device);
	UINT32 val = k037122->tile_ram[tile_index + (0x8000/4)];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_DEVICE(k037122->gfx_index, tile, color, flags);
}

static TILE_GET_INFO_DEVICE( k037122_tile_info_layer1 )
{
	k037122_state *k037122 = k037122_get_safe_token(device);
	UINT32 val = k037122->tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_DEVICE(k037122->gfx_index, tile, color, flags);
}


void k037122_tile_draw( const device_config *device, bitmap_t *bitmap, const rectangle *cliprect )
{
	k037122_state *k037122 = k037122_get_safe_token(device);
	const rectangle *visarea = video_screen_get_visible_area(k037122->screen);

	if (k037122->reg[0xc] & 0x10000)
	{
		tilemap_set_scrolldx(k037122->layer[1], visarea->min_x, visarea->min_x);
		tilemap_set_scrolldy(k037122->layer[1], visarea->min_y, visarea->min_y);
		tilemap_draw(bitmap, cliprect, k037122->layer[1], 0, 0);
	}
	else
	{
		tilemap_set_scrolldx(k037122->layer[0], visarea->min_x, visarea->min_x);
		tilemap_set_scrolldy(k037122->layer[0], visarea->min_y, visarea->min_y);
		tilemap_draw(bitmap, cliprect, k037122->layer[0], 0, 0);
	}
}

static void update_palette_color( const device_config *device, UINT32 palette_base, int color )
{
	k037122_state *k037122 = k037122_get_safe_token(device);
	UINT32 data = k037122->tile_ram[(palette_base / 4) + color];

	palette_set_color_rgb(device->machine, color, pal5bit(data >> 6), pal6bit(data >> 0), pal5bit(data >> 11));
}

READ32_DEVICE_HANDLER( k037122_sram_r )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);

	return k037122->tile_ram[offset];
}

WRITE32_DEVICE_HANDLER( k037122_sram_w )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);

	COMBINE_DATA(k037122->tile_ram + offset);

	if (k037122->reg[0xc] & 0x10000)
	{
		if (offset < 0x8000 / 4)
		{
			tilemap_mark_tile_dirty(k037122->layer[1], offset);
		}
		else if (offset >= 0x8000 / 4 && offset < 0x18000 / 4)
		{
			tilemap_mark_tile_dirty(k037122->layer[0], offset - (0x8000 / 4));
		}
		else if (offset >= 0x18000 / 4)
		{
			update_palette_color(device, 0x18000, offset - (0x18000 / 4));
		}
	}
	else
	{
		if (offset < 0x8000 / 4)
		{
			update_palette_color(device, 0, offset);
		}
		else if (offset >= 0x8000 / 4 && offset < 0x18000 / 4)
		{
			tilemap_mark_tile_dirty(k037122->layer[0], offset - (0x8000 / 4));
		}
		else if (offset >= 0x18000 / 4)
		{
			tilemap_mark_tile_dirty(k037122->layer[1], offset - (0x18000 / 4));
		}
	}
}


READ32_DEVICE_HANDLER( k037122_char_r )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);
	int bank = k037122->reg[0x30 / 4] & 0x7;

	return k037122->char_ram[offset + (bank * (0x40000 / 4))];
}

WRITE32_DEVICE_HANDLER( k037122_char_w )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);
	int bank = k037122->reg[0x30 / 4] & 0x7;
	UINT32 addr = offset + (bank * (0x40000/4));

	COMBINE_DATA(k037122->char_ram + addr);
	gfx_element_mark_dirty(device->machine->gfx[k037122->gfx_index], addr / 32);
}

READ32_DEVICE_HANDLER( k037122_reg_r )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);

	switch (offset)
	{
		case 0x14/4:
		{
			return 0x000003fa;
		}
	}
	return k037122->reg[offset];
}

WRITE32_DEVICE_HANDLER( k037122_reg_w )
{
//  int chip = get_cgboard_id();
	k037122_state *k037122 = k037122_get_safe_token(device);

	COMBINE_DATA(k037122->reg + offset);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k037122 )
{
	k037122_state *k037122 = k037122_get_safe_token(device);
	const k037122_interface *intf = k037122_get_interface(device);

	k037122->screen = devtag_get_device(device->machine, intf->screen);
	k037122->gfx_index = intf->gfx_index;

	k037122->char_ram = auto_alloc_array(device->machine, UINT32, 0x200000 / 4);
	k037122->tile_ram = auto_alloc_array(device->machine, UINT32, 0x20000 / 4);
	k037122->reg = auto_alloc_array(device->machine, UINT32, 0x400 / 4);

	k037122->layer[0] = tilemap_create_device(device, k037122_tile_info_layer0, tilemap_scan_rows, 8, 8, 256, 64);
	k037122->layer[1] = tilemap_create_device(device, k037122_tile_info_layer1, tilemap_scan_rows, 8, 8, 128, 64);

	tilemap_set_transparent_pen(k037122->layer[0], 0);
	tilemap_set_transparent_pen(k037122->layer[1], 0);

	device->machine->gfx[k037122->gfx_index] = gfx_element_alloc(device->machine, &k037122_char_layout, (UINT8*)k037122->char_ram, device->machine->config->total_colors / 16, 0);

	state_save_register_device_item_pointer(device, 0, k037122->reg, 0x400 / 4);
	state_save_register_device_item_pointer(device, 0, k037122->char_ram, 0x200000 / 4);
	state_save_register_device_item_pointer(device, 0, k037122->tile_ram, 0x20000 / 4);
}

static DEVICE_RESET( k037122 )
{
	k037122_state *k037122 = k037122_get_safe_token(device);

	memset(k037122->char_ram, 0, 0x200000);
	memset(k037122->tile_ram, 0, 0x20000);
	memset(k037122->reg, 0, 0x400);
}


/***************************************************************************/
/*                                                                         */
/*                         misc debug handlers                             */
/*                                                                         */
/***************************************************************************/

READ16_DEVICE_HANDLER( k056832_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return (k056832->regs[offset]);
}		// VACSET

READ16_DEVICE_HANDLER( k056832_b_word_r )
{
	k056832_state *k056832 = k056832_get_safe_token(device);
	return (k056832->regsb[offset]);
}	// VSCCS (board dependent)

READ16_DEVICE_HANDLER( k053246_reg_word_r )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return(k053247->kx46_regs[offset * 2] << 8 | k053247->kx46_regs[offset * 2 + 1]);
}	// OBJSET1

READ16_DEVICE_HANDLER( k053247_reg_word_r )
{
	k053247_state *k053247 = k053247_get_safe_token(device);
	return(k053247->kx47_regs[offset]);
}	// OBJSET2

READ16_DEVICE_HANDLER( k054338_word_r )
{
	k054338_state *k054338 = k054338_get_safe_token(device);
	return(k054338->regs[offset]);
}		// CLTC

READ16_DEVICE_HANDLER( k053251_lsb_r )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	return(k053251->ram[offset]);
}		// PCU1

READ16_DEVICE_HANDLER( k053251_msb_r )
{
	k053251_state *k053251 = k053251_get_safe_token(device);
	return(k053251->ram[offset] << 8);
}		// PCU1

READ16_DEVICE_HANDLER( k055555_word_r )
{
	k055555_state *k055555 = k055555_get_safe_token(device);
	return(k055555->regs[offset] << 8);
}	// PCU2

READ32_DEVICE_HANDLER( k056832_long_r )
{
	offset <<= 1;
	return (k056832_word_r(device, offset + 1, 0xffff) | k056832_word_r(device, offset, 0xffff) << 16);
}

READ32_DEVICE_HANDLER( k053247_reg_long_r )
{
	offset <<= 1;
	return (k053247_reg_word_r(device, offset + 1, 0xffff) | k053247_reg_word_r(device, offset, 0xffff) << 16);
}

READ32_DEVICE_HANDLER( k055555_long_r )
{
	offset <<= 1;
	return (k055555_word_r(device, offset + 1, 0xffff) | k055555_word_r(device, offset, 0xffff) << 16);
}

READ16_DEVICE_HANDLER( k053244_reg_word_r )
{
	k05324x_state *k053244 = k05324x_get_safe_token(device);
	return(k053244->regs[offset * 2] << 8 | k053244->regs[offset * 2 + 1]);
}



/***************************************************************************/
/*                                                                         */
/*                         DEVICE_GET_INFOs                                */
/*                                                                         */
/***************************************************************************/


DEVICE_GET_INFO( k007121 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k007121_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k007121);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k007121);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 007121");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k007342 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k007342_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k007342);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k007342);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 007342");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k007420 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k007420_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k007420);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k007420);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 007420");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k052109 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k052109_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k052109);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k052109);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 052109");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k051960 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k051960_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k051960);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k051960);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 051960");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k05324x )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k05324x_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k05324x);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k05324x);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053244 & 053245");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k053247 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053247_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k053247);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053247);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053246 & 053247");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k055673 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053247_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k055673);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053247);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 055673");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k051316 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k051316_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k051316);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k051316);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 051316");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k053936 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053936_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k053936);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053936);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053936");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k053251 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053251_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k053251);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053251);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053251");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k054000 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k054000_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k054000);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k054000);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 054000");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k051733 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k051733_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k051733);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k051733);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 051733");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k056832 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k056832_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k056832);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */						break;
		case DEVINFO_FCT_RESET:					/* Nothing */						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 056832");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k055555 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k055555_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k055555);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k055555);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 055555");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k054338 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k054338_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k054338);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k054338);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 054338");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k053250 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053250_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k053250);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053250);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053250");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k053252 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k053252_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k053252);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k053252);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 053252");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( k001006 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k001006_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k001006);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k001006);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 001006");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k001005 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k001005_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k001005);		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(k001005);		break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k001005);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 001005");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k001604 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k001604_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k001604);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k001604);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 001604");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEVICE_GET_INFO( k037122 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(k037122_state);					break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(k037122);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(k037122);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Konami 0371222");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Konami Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}
