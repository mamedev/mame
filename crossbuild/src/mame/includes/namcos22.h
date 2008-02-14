#define USE_NAMCOS22_SPEED_HACK

#define REGION_CGRAM           REGION_GFX1
#define REGION_TEXTURE_TILE    REGION_GFX2
#define REGION_TEXTURE_TILEMAP REGION_GFX3
#define REGION_POINTROM        REGION_GFX4
#define REGION_SPRITE          REGION_GFX5

#define GFX_CHAR               0
#define GFX_TEXTURE_TILE       1
#define GFX_SPRITE             2

extern enum namcos22_gametype
{
	NAMCOS22_AIR_COMBAT22,
	NAMCOS22_ALPINE_RACER,
	NAMCOS22_CYBER_COMMANDO,
	NAMCOS22_CYBER_CYCLES,
	NAMCOS22_PROP_CYCLE,
	NAMCOS22_RAVE_RACER,
	NAMCOS22_RIDGE_RACER,
	NAMCOS22_RIDGE_RACER2,
	NAMCOS22_TIME_CRISIS,
	NAMCOS22_VICTORY_LAP,
	NAMCOS22_ACE_DRIVER,
	NAMCOS22_ALPINE_RACER_2,
	NAMCOS22_ALPINE_SURFER
} namcos22_gametype;

#define NAMCOS22_NUM_ROWS 30
#define NAMCOS22_NUM_COLS 40

#define NAMCOS22_PALETTE_SIZE 0x8000

/*----------- defined in drivers/namcos22.c -----------*/

extern const gfx_layout namcos22_cg_layout;

/*----------- defined in video/namcos22.c -----------*/

extern UINT32 *namcos22_cgram;
extern UINT32 *namcos22_textram;
extern UINT32 *namcos22_polygonram;
extern UINT32 *namcos22_gamma;
extern UINT32 *namcos22_vics_data;
extern UINT32 *namcos22_vics_control;
extern UINT32 *namcos22_czattr;
extern UINT32 *namcos22_tilemapattr;

WRITE16_HANDLER( namcos22_dspram16_bank_w );
READ16_HANDLER( namcos22_dspram16_r );
WRITE16_HANDLER( namcos22_dspram16_w );

READ32_HANDLER( namcos22_cgram_r );
WRITE32_HANDLER( namcos22_cgram_w );

READ32_HANDLER( namcos22_czram_r );
WRITE32_HANDLER( namcos22_czram_w );

WRITE32_HANDLER(namcos22_port800000_w);

READ32_HANDLER( namcos22_paletteram_r );
WRITE32_HANDLER( namcos22_paletteram_w );

READ32_HANDLER( namcos22_textram_r );
WRITE32_HANDLER( namcos22_textram_w );

READ32_HANDLER( namcos22_gamma_r );
WRITE32_HANDLER( namcos22_gamma_w );

READ32_HANDLER( namcos22_dspram_r );
WRITE32_HANDLER( namcos22_dspram_w );


VIDEO_START( namcos22 );
VIDEO_UPDATE( namcos22 );

VIDEO_START( namcos22s );
VIDEO_UPDATE( namcos22s );

void namcos22_draw_direct_poly( const UINT16 *pSource );
extern UINT32 namcos22_point_rom_r( offs_t offs );
extern void namcos22_enable_slave_simulation( void );
