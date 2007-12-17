/* namcoic.h

Custom Chips:                       Final Lap   Assault     LuckyWld    System21    NA1/2       NB1/2
    C45     Land Generator            *                       *
    C65     I/O Controller            *           *
    C68                                                       *           *
    C70                                                                               *
    C95                               *           *
    C102    ROZ:Memory Access Control             *
    C106    OBJ:X-Axis Zoom Control   *           *
    C107    Land Line Buffer          *
    C116    Screen Waveform Generator *           *           *                                   *
    C121    Yamaha YM2151 Sound Gen   *           *           *
    C123    GFX:Tile Mem Decoder      *           *           *                                   *
    C134    OBJ:Address Generator     *           *
    C135    OBJ:Line matching         *           *
    C137    Clock Generator IC        *           *           *           *                       *
    C138                                                                  *
    C139    Serial I/F Controller     *           *           *           *
    C140    24 Channel PCM            *           *           *
    C145    GFX:Tile Memory Access    *           *           *                                   *
    C146    OBJ:Line Buf Steering     *           *
    C148    CPU Bus Manager           *           *           *           *
    C149    Mouse/Trackball Decoder   *           *           *           *
    C156    Pixel Stream Combo        *           *           *                                   *
    C160    Control                                                                               *
    C165                                                                  *
    C169    ROZ(B)                                            *                                   *
    C187                                                      *           *                       *
    C210                                                                              *
    C215                                                                              *
    C218                                                                              *
    C219                                                                              *
    C329    CPU?                                                                                  *
    C347    GfxObj                                                                                *
    C352    PCM                                                                                   *
    C355    Motion Obj(B)                                     *           *                       *
    C373    LAND-related                                      *
    C382                                                                                          *
    C383                                                                                          *
    C384    GFX(3)                                                                                *
    C385                                                                                          *
    C390    Key Custom                                                                            *


General Support
---------------
C65  - This is the I/O Microcontroller, handles all input/output devices. 63705 uC, CPU4 in Namco System2.
C137 - Takes System clock and generates all sub-system clocks, doesnt need emulation, not accessed via CPU
C139 - Serial Interface Controller
C148 - Does some Memory Decode, Interrupt Handling, 3 bit PIO port, Bus Controller
C149 - Does decoding of mouse/trackball input streams for the I/O Controller. (Offset Square wave)


Tile Fields Static/Scrolled
---------------------------
Combination of these two devices and associated RAM & TileGFX produces a pixel stream that is fed
into the Pixel stream decoder.

C145 - Tile Screen Memory Access controller
C123 - Tile Memory decoder Part 1, converts X,Y,Tile into character ROM address index


Pixel Stream Decode
-------------------
These two devices take the pixel streams from the tilefield generator and the associated graphics board
and combine them to form an RGB data stream that is fed to the monitor.

C156 - Pixel stream combiner
Takes tile field & graphics board streams and generates the priorisied pixel, then does the lookup to
go from palettised to 24bit RGB pixel.

C116 - Screen Waveform Generator
Takes RGB24 pixel stream from C156 and generates the waveform signals for the monitor, also generates
the line interrupt and controls screen blanking,shift, etc.

Object Control
--------------
C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
C135 - Checks is object is displayed on Current output line.
C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

ROZ
---
C102 - Controls CPU access to ROZ Memory Area.
*/

/***********************************************************************************/

/*----------- defined in drivers/namcoic.c -----------*/

void namco_tilemap_init(
		int gfxbank, void *pMaskROM,
		void (*cb)( UINT16 code, int *gfx, int *mask) );
void namco_tilemap_draw( mame_bitmap *bitmap, const rectangle *cliprect, int pri );
void namco_tilemap_invalidate( void );
WRITE16_HANDLER( namco_tilemapvideoram16_w );
READ16_HANDLER( namco_tilemapvideoram16_r );
WRITE16_HANDLER( namco_tilemapcontrol16_w );
READ16_HANDLER( namco_tilemapcontrol16_r );

READ32_HANDLER( namco_tilemapvideoram32_r );
WRITE32_HANDLER( namco_tilemapvideoram32_w );
READ32_HANDLER( namco_tilemapcontrol32_r );
WRITE32_HANDLER( namco_tilemapcontrol32_w );

READ32_HANDLER( namco_tilemapvideoram32_le_r );
WRITE32_HANDLER( namco_tilemapvideoram32_le_w );
READ32_HANDLER( namco_tilemapcontrol32_le_r );
WRITE32_HANDLER( namco_tilemapcontrol32_le_w );

/***********************************************************************************/

/* Namco System II Sprite Rendering */
void namcos2_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri, int control );

void namcos2_draw_sprites_metalhawk(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri );

/***********************************************************************************/
/* C355 Motion Object Emulation */

/* for palXOR, supply either 0x0 (normal) or 0xf (palette mapping reversed) */
void namco_obj_init( int gfxbank, int palXOR, int (*code2tile)( int code ) );
void namco_obj_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri );

WRITE16_HANDLER( namco_obj16_w );
READ16_HANDLER( namco_obj16_r );

WRITE32_HANDLER( namco_obj32_w );
READ32_HANDLER( namco_obj32_r );
WRITE32_HANDLER( namco_obj32_le_w );
READ32_HANDLER( namco_obj32_le_r );

WRITE16_HANDLER( namco_spritepos16_w );
READ16_HANDLER( namco_spritepos16_r );

WRITE32_HANDLER( namco_spritepos32_w );
READ32_HANDLER( namco_spritepos32_r );

/***********************************************************************************/
/* C169 ROZ Layer Emulation */

void namco_roz_init( int gfxbank, int maskregion );
void namco_roz_draw( mame_bitmap *bitmap, const rectangle *cliprect, int pri );

READ16_HANDLER( namco_rozcontrol16_r );
WRITE16_HANDLER( namco_rozcontrol16_w );
READ16_HANDLER( namco_rozbank16_r );
WRITE16_HANDLER( namco_rozbank16_w );
READ16_HANDLER( namco_rozvideoram16_r );
WRITE16_HANDLER( namco_rozvideoram16_w );

READ32_HANDLER( namco_rozcontrol32_r );
WRITE32_HANDLER( namco_rozcontrol32_w );
READ32_HANDLER( namco_rozcontrol32_le_r );
WRITE32_HANDLER( namco_rozcontrol32_le_w );
READ32_HANDLER( namco_rozbank32_r );
WRITE32_HANDLER( namco_rozbank32_w );
READ32_HANDLER( namco_rozvideoram32_r );
WRITE32_HANDLER( namco_rozvideoram32_w );
READ32_HANDLER( namco_rozvideoram32_le_r );
WRITE32_HANDLER( namco_rozvideoram32_le_w );

/***********************************************************************************/
/* C45 Land (Road) Emulation */

void namco_road_init(running_machine *machine, int gfxbank );
void namco_road_set_transparent_color(pen_t pen);
void namco_road_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri );

READ16_HANDLER( namco_road16_r );
WRITE16_HANDLER( namco_road16_w );

/***********************************************************************************/
