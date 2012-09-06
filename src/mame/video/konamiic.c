/***************************************************************************

TODO:
- It seems shadows can both affect underlying sprites and not. This is currently
  hardcoded in the drivers; there might be a control bit somewhere.
  Games requiring shadows to affect sprites behind them:
  - Surprise Attack (dark glass walls in level 3)
  - 88 Games (angle indicator in the long jump event)
  - Sunset Riders (bull's eye in the saloon cutscene)
  - TMNT 2 (lightbeam in level 4 cave)
  - Metamorphic Force (double! lightbeam just before the last boss)
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
Contra / Gryzor     GX633*1987    6309           007121(x2)               007593
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
Over Drive          pass
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
     ------x- enable rowscroll? (combatsc)
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
              visible area from 256 to 240 pixels. This is used by combatsc on
              the scrolling stages, and by labyrunr on the title screen.
              At first I thought that this enabled an extra bank of 0x40
              sprites, needed by combatsc, but labyrunr proves that this is not
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
     -------x bit 3 of attribute is bit 3 of color (combatsc, fastlane, flkatck)
     ------x- bit 4 of attribute is tile flip X (assumption - no game uses this)
     -----x-- bit 5 of attribute is tile flip Y (flkatck)
     ----x--- bit 6 of attribute is tile priority over sprites (combatsc, hcastle,
              labyrunr)
              Note that hcastle sets this bit for layer 0, and bit 6 of the
              attribute is also used as bit 12 of the tile code, however that
              bit is ALWAYS set throughout the game.
              combatsc uses the bit in the "graduation" scene during attract mode,
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
1e00     : ROM membank selector for ROM testing
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

#include "emu.h"
#include "video/konamiic.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



static void decode_gfx(running_machine &machine, int gfx_index, UINT8 *data, UINT32 total, const gfx_layout *layout, int bpp)
{
	gfx_layout gl;

	memcpy(&gl, layout, sizeof(gl));
	gl.total = total;
	machine.gfx[gfx_index] = auto_alloc(machine, gfx_element(machine, gl, data, machine.total_colors() >> bpp, 0));
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
static void (*K053247_callback)(running_machine &machine, int *code,int *color,int *priority);
static UINT8 K053246_OBJCHA_line;

void K053247_export_config(UINT16 **ram, gfx_element **gfx, void (**callback)(running_machine &, int *, int *, int *), int *dx, int *dy)
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


/* K055673 used with the 54246 in PreGX/Run and Gun/System GX games */
void K055673_vh_start(running_machine &machine, const char *gfx_memory_region, int layout, int dx, int dy, void (*callback)(running_machine &machine, int *code,int *color,int *priority))
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
		if (machine.gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	K055673_rom = (UINT16 *)machine.root_device().memregion(gfx_memory_region)->base();

	/* decode the graphics */
	switch(layout)
	{
	case K055673_LAYOUT_GX:
		size4 = (machine.root_device().memregion(gfx_memory_region)->bytes()/(1024*1024))/5;
		size4 *= 4*1024*1024;
		/* set the # of tiles based on the 4bpp section */
		K055673_rom = auto_alloc_array(machine, UINT16, size4 * 5 / 2);
		d = (UINT8 *)K055673_rom;
		// now combine the graphics together to form 5bpp
		s1 = machine.root_device().memregion(gfx_memory_region)->base(); // 4bpp area
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
		total = machine.root_device().memregion(gfx_memory_region)->bytes() / (16*16/2);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout2, 4);
		break;

	case K055673_LAYOUT_LE2:
		total = machine.root_device().memregion(gfx_memory_region)->bytes() / (16*16);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout3, 4);
		break;

	case K055673_LAYOUT_GX6:
		total = machine.root_device().memregion(gfx_memory_region)->bytes() / (16*16*6/8);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout4, 4);
		break;

	default:
		fatalerror("Unsupported layout");
	}

	if (VERBOSE && !(machine.config().m_video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	K053247_dx = dx;
	K053247_dy = dy;
	K053247_wraparound = 1;
	K05324x_z_rejection = -1;
	K053247_memory_region = gfx_memory_region;
	K053247_gfx = machine.gfx[gfx_index];
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

// Mystic Warriors hardware games support a non-OBJCHA based ROM readback
// write the address to the 246 as usual, but there's a completely separate ROM
// window that works without needing an OBJCHA line.
// in this window, +0 = 32 bits from one set of ROMs, and +8 = 32 bits from another set
READ16_HANDLER( K055673_rom_word_r )	// 5bpp
{
	UINT8 *ROM8 = (UINT8 *)space->machine().root_device().memregion(K053247_memory_region)->base();
	UINT16 *ROM = (UINT16 *)space->machine().root_device().memregion(K053247_memory_region)->base();
	int size4 = (space->machine().root_device().memregion(K053247_memory_region)->bytes()/(1024*1024))/5;
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
	UINT16 *ROM = (UINT16 *)space->machine().root_device().memregion(K053247_memory_region)->base();
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
			LOG(("55673_rom_word_r: Unknown read offset %x (PC=%x)\n", offset, cpu_get_pc(&space->device())));
			break;
	}

	return 0;
}

static WRITE8_HANDLER( K053246_w )
{
	K053246_regs[offset] = data;
}


WRITE16_HANDLER( K053246_word_w )
{
	if (ACCESSING_BITS_8_15)
		K053246_w(space, offset<<1,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K053246_w(space, (offset<<1) + 1,data & 0xff);
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

static void K053936_zoom_draw(int chip,UINT16 *ctrl,UINT16 *linectrl, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
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
			if (my_clip.min_x < cliprect.min_x)
				my_clip.min_x = cliprect.min_x;
			if (my_clip.max_x > cliprect.max_x)
				my_clip.max_x = cliprect.max_x;

			y = ctrl[0x0a] + K053936_offset[chip][1]-2;
			if (y < cliprect.min_y)
				y = cliprect.min_y;
			maxy = ctrl[0x0b] + K053936_offset[chip][1]-2 - 1;
			if (maxy > cliprect.max_y)
				maxy = cliprect.max_y;
		}
		else
		{
			my_clip.min_x = cliprect.min_x;
			my_clip.max_x = cliprect.max_x;

			y = cliprect.min_y;
			maxy = cliprect.max_y;
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

			tmap->draw_roz(bitmap, my_clip, startx << 5,starty << 5,
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

		tmap->draw_roz(bitmap, cliprect, startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				K053936_wraparound[chip],
				flags,priority);
	}

#if 0
if (machine.input().code_pressed(KEYCODE_D))
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


void K053936_0_zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	K053936_zoom_draw(0,K053936_0_ctrl,K053936_0_linectrl,bitmap,cliprect,tmap,flags,priority, glfgreat_hack);
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
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 K054000_ram[0x20];

static WRITE8_HANDLER( K054000_w )
{
//logerror("%04x: write %02x to 054000 address %02x\n",cpu_get_pc(&space->device()),data,offset);

	K054000_ram[offset] = data;
}

static READ8_HANDLER( K054000_r )
{
	int Acx,Acy,Aax,Aay;
	int Bcx,Bcy,Bax,Bay;

//logerror("%04x: read 054000 address %02x\n",cpu_get_pc(&space->device()),offset);

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
/*                                 054157 / 056832                         */
/*                                                                         */
/***************************************************************************/

#define K056832_PAGE_COLS 64
#define K056832_PAGE_ROWS 32
#define K056832_PAGE_HEIGHT (K056832_PAGE_ROWS*8)
#define K056832_PAGE_WIDTH  (K056832_PAGE_COLS*8)
#define K056832_PAGE_COUNT 16

static tilemap_t *K056832_tilemap[K056832_PAGE_COUNT];
static bitmap_ind16 *K056832_pixmap[K056832_PAGE_COUNT];

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
		K056832_tilemap[page]->mark_all_dirty();
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

	// winning spike and vsnet soccer don't like our layer association implementation..
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

static void (*K056832_callback)(running_machine &machine, int layer, int *code, int *color, int *flags);

INLINE void K056832_get_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int pageIndex )
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

	(*K056832_callback)(machine, layer, &code, &color, &flags);

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

static void K056832_postload(running_machine &machine)
{
	K056832_UpdatePageLayout();
	K056832_change_rambank();
	K056832_change_rombank();
}

void K056832_vh_start(running_machine &machine, const char *gfx_memory_region, int bpp, int big,
	int (*scrolld)[4][2],
	void (*callback)(running_machine &machine, int layer, int *code, int *color, int *flags),
	int djmain_hack)
{
	tilemap_t *tmap;
	int gfx_index;
	int i;
	UINT32 total;
	static const gfx_layout charlayout8_tasman =
	{
		8,8,
		RGN_FRAC(1,1),
		8,
		{ 0,8,16,24,32,40,48,56 },
		{ 0,1,2,3,4,5,6,7 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64},
		8*64
	};
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
		if (machine.gfx[gfx_index] == 0) break;
	}
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* handle the various graphics formats */
	i = (big) ? 8 : 16;

	/* decode the graphics */
	switch (bpp)
	{
		case K056832_BPP_4:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*4);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout4, 4);
			break;

		case K056832_BPP_5:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*5);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout5, 4);
			break;

		case K056832_BPP_6:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*6);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout6, 4);
			break;

		case K056832_BPP_8:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout8, 4);
			break;

		case K056832_BPP_8LE:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout8le, 4);
			break;

		case K056832_BPP_8TASMAN:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout8_tasman, 4);
			break;

		case K056832_BPP_4dj:
			total = machine.root_device().memregion(gfx_memory_region)->bytes() / (i*4);
			decode_gfx(machine, gfx_index, machine.root_device().memregion(gfx_memory_region)->base(), total, &charlayout4dj, 4);
			break;

		default:
			fatalerror("Unsupported bpp");
	}

	machine.gfx[gfx_index]->set_granularity(16); /* override */

	K056832_memory_region = gfx_memory_region;
	K056832_gfxnum = gfx_index;
	K056832_callback = callback;

	K056832_rombase = machine.root_device().memregion(gfx_memory_region)->base();
	K056832_NumGfxBanks = machine.root_device().memregion(gfx_memory_region)->bytes() / 0x2000;
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

	K056832_tilemap[0x0] = tilemap_create(machine, K056832_get_tile_info0, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x1] = tilemap_create(machine, K056832_get_tile_info1, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x2] = tilemap_create(machine, K056832_get_tile_info2, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x3] = tilemap_create(machine, K056832_get_tile_info3, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x4] = tilemap_create(machine, K056832_get_tile_info4, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x5] = tilemap_create(machine, K056832_get_tile_info5, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x6] = tilemap_create(machine, K056832_get_tile_info6, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x7] = tilemap_create(machine, K056832_get_tile_info7, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x8] = tilemap_create(machine, K056832_get_tile_info8, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0x9] = tilemap_create(machine, K056832_get_tile_info9, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xa] = tilemap_create(machine, K056832_get_tile_infoa, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xb] = tilemap_create(machine, K056832_get_tile_infob, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xc] = tilemap_create(machine, K056832_get_tile_infoc, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xd] = tilemap_create(machine, K056832_get_tile_infod, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xe] = tilemap_create(machine, K056832_get_tile_infoe, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	K056832_tilemap[0xf] = tilemap_create(machine, K056832_get_tile_infof, TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	for (i=0; i<K056832_PAGE_COUNT; i++)
	{
		tmap = K056832_tilemap[i];

		K056832_pixmap[i] = &tmap->pixmap();

		tmap->set_transparent_pen(0);
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

	machine.save().register_postload(save_prepost_delegate(FUNC(K056832_postload), &machine));
}



/* generic helper routine for ROM checksumming */
static int K056832_rom_read_b(running_machine &machine, int offset, int blksize, int blksize2, int zerosec)
{
	UINT8 *rombase;
	int base, ret;

	rombase = (UINT8 *)machine.root_device().memregion(K056832_memory_region)->base();

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


READ32_HANDLER( K056832_5bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
	{
		return K056832_rom_read_b(space->machine(), offset*4, 4, 5, 0)<<24;
	}
	else if (mem_mask == 0x00ff0000)
	{
		return K056832_rom_read_b(space->machine(), offset*4+1, 4, 5, 0)<<16;
	}
	else if (mem_mask == 0x0000ff00)
	{
		return K056832_rom_read_b(space->machine(), offset*4+2, 4, 5, 0)<<8;
	}
	else if (mem_mask == 0x000000ff)
	{
		return K056832_rom_read_b(space->machine(), offset*4+3, 4, 5, 1);
	}
	else
	{
		LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(&space->device()), mem_mask));
	}
	return 0;
}

READ32_HANDLER( K056832_6bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
	{
		return K056832_rom_read_b(space->machine(), offset*4, 4, 6, 0)<<24;
	}
	else if (mem_mask == 0x00ff0000)
	{
		return K056832_rom_read_b(space->machine(), offset*4+1, 4, 6, 0)<<16;
	}
	else if (mem_mask == 0x0000ff00)
	{
		return K056832_rom_read_b(space->machine(), offset*4+2, 4, 6, 0)<<8;
	}
	else if (mem_mask == 0x000000ff)
	{
		return K056832_rom_read_b(space->machine(), offset*4+3, 4, 6, 0);
	}
	else
	{
		LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(&space->device()), mem_mask));
	}
	return 0;
}


// data is arranged like this:
// 0000 1111 22 0000 1111 22
READ16_HANDLER( K056832_mw_rom_word_r )
{
	int bank = 10240*K056832_CurGfxBank;
	int addr;

	if (!K056832_rombase)
	{
		K056832_rombase = space->machine().root_device().memregion(K056832_memory_region)->base();
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


/* only one page is mapped to videoram at a time through a window */
READ16_HANDLER( K056832_ram_word_r )
{
	// reading from tile RAM resets the ROM readback "half" offset
	K056832_rom_half = 0;

	return K056832_videoram[K056832_SelectedPagex4096+offset];
}


READ32_HANDLER( K056832_ram_long_r )
{
	UINT16 *pMem = &K056832_videoram[K056832_SelectedPagex4096+offset*2];

	// reading from tile RAM resets the ROM readback "half" offset
	K056832_rom_half = 0;

	return(pMem[0]<<16 | pMem[1]);
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
			K056832_tilemap[K056832_SelectedPage]->mark_tile_dirty(offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
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
			K056832_tilemap[K056832_SelectedPage]->mark_tile_dirty(offset);
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
						K056832_tilemap[i]->set_flip(flip);
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


static int K056832_update_linemap(running_machine &machine, bitmap_rgb32 &bitmap, int page, int flags)
{

	if (K056832_PageTileMode[page]) return(0);
	if (!K056832_linemap_enabled) return(1);


	{

		rectangle zerorect;
		tilemap_t *tmap;
		UINT32 *dirty;
		int all_dirty;
		UINT8 *xprdata;

		tmap = K056832_tilemap[page];
		bitmap_ind8 &xprmap  = tmap->flagsmap();
		xprdata = tmap->tile_flags();

		dirty = K056832_LineDirty[page];
		all_dirty = K056832_AllLinesDirty[page];

		if (all_dirty)
		{
			dirty[7]=dirty[6]=dirty[5]=dirty[4]=dirty[3]=dirty[2]=dirty[1]=dirty[0] = 0;
			K056832_AllLinesDirty[page] = 0;

			// force tilemap into a clean, static state
			// *really ugly but it minimizes alteration to tilemap.c
			memset (&zerorect, 0, sizeof(rectangle));	// zero dimension
			tmap->draw(bitmap, zerorect, 0, 0);	// dummy call to reset tile_dirty_map
			xprmap.fill(0);						// reset pixel transparency_bitmap;
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


			bitmap_ind16 *pixmap;

			UINT8 code_transparent, code_opaque;
			const pen_t *pal_ptr;
			const UINT8  *src_ptr;
			UINT8  *xpr_ptr;
			UINT16 *dst_ptr;
			UINT16 pen, basepen;
			int count, src_pitch, src_modulo;
			int	dst_pitch;
			int line;
			gfx_element *src_gfx;
			int offs, mask;

			#define LINE_WIDTH 512

			#define DRAW_PIX(N) \
				pen = src_ptr[N]; \
				if (pen) \
				{ pen += basepen; xpr_ptr[count+N] = TILEMAP_PIXEL_LAYER0; dst_ptr[count+N] = pen; } else \
				{ xpr_ptr[count+N] = 0; }

			pixmap  = K056832_pixmap[page];
			pal_ptr    = machine.pens;
			src_gfx    = machine.gfx[K056832_gfxnum];
			src_pitch  = src_gfx->rowbytes();
			src_modulo = src_gfx->char_modulo;
			dst_pitch  = pixmap->rowpixels;

			for (line=0; line<256; line++)
			{


				tile_data tileinfo = {0};

				dst_ptr = &pixmap->pix16(line);
				xpr_ptr = &xprmap.pix8(line);

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

void K056832_tilemap_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority)
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
	tilemap_t *tmap;
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

	cminx = cliprect.min_x;
	cmaxx = cliprect.max_x;
	cminy = cliprect.min_y;
	cmaxy = cliprect.max_y;

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
		tmap->set_scrolly(0, ay);

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
			if ((drawrect.min_x>0) && (drawrect.max_x==511)) drawrect.max_x=cliprect.max_x;

			tmap->set_scrollx(0, dx);

			LINE_SHORTCIRCUIT:
			tmap->draw(bitmap, drawrect, flags, priority);

		} // end of line loop
	} // end of column loop
  } // end of row loop

	K056832_ActiveLayer = last_active;

} // end of function


int K056832_get_LayerAssociation(void)
{
	return(K056832_LayerAssociation);
}

void K056832_set_LayerOffset(int layer, int offsx, int offsy)
{
	K056832_LayerOffset[layer][0] = offsx;
	K056832_LayerOffset[layer][1] = offsy;
}


void K056832_set_UpdateMode(int mode)
{
	K056832_UpdateMode = mode;
}




/***************************************************************************/
/*                                                                         */
/*                                 055555                                  */
/*                                                                         */
/***************************************************************************/

/* K055555 5-bit-per-pixel priority encoder */
/* This device has 48 8-bit-wide registers */

static UINT8 k55555_regs[128];

void K055555_vh_start(running_machine &machine)
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
void K054338_vh_start(running_machine &machine)
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

void K054338_update_all_shadows(running_machine &machine, int rushingheroes_hack)
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

#ifdef UNUSED_FUNCTION
// K054338 BG color fill
void K054338_fill_solid_bg(bitmap_ind16 &bitmap)
{
	UINT32 bgcolor;
	UINT32 *pLine;
	int x, y;

	bgcolor = (K054338_read_register(K338_REG_BGC_R)&0xff)<<16;
	bgcolor |= K054338_read_register(K338_REG_BGC_GB);

	/* and fill the screen with it */
	for (y = 0; y < bitmap.height; y++)
	{
		pLine = (UINT32 *)bitmap.base;
		pLine += (bitmap.rowpixels*y);
		for (x = 0; x < bitmap.width; x++)
			*pLine++ = bgcolor;
	}
}
#endif

// Unified K054338/K055555 BG color fill
void K054338_fill_backcolor(running_machine &machine, bitmap_rgb32 &bitmap, int mode) // (see p.67)
{
	int clipx, clipy, clipw, cliph, i, dst_pitch;
	int BGC_CBLK, BGC_SET;
	UINT32 *dst_ptr, *pal_ptr;
	int bgcolor;
	const rectangle &visarea = machine.primary_screen->visible_area();
	driver_device *state = machine.driver_data();

	clipx = visarea.min_x & ~3;
	clipy = visarea.min_y;
	clipw = (visarea.max_x - clipx + 4) & ~3;
	cliph = visarea.max_y - clipy + 1;

	dst_ptr = &bitmap.pix32(clipy);
	dst_pitch = bitmap.rowpixels();
	dst_ptr += clipx;

	BGC_SET = 0;
	pal_ptr = state->m_generic_paletteram_32;

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
/*                                 053252                                  */
/*                                                                         */
/***************************************************************************/

// K053252 CRT and interrupt control unit
static UINT16 K053252_regs[16];

WRITE16_HANDLER( K053252_word_w )
{
	COMBINE_DATA(K053252_regs + offset);
}






