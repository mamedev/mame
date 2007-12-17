/*----------- defined in drivers/namconb1.c -----------*/

extern UINT32 *namconb1_workram32;
extern UINT32 *namconb1_spritebank32;
extern UINT32 *namconb1_tilebank32;

/*----------- defined in video/namconb1.c -----------*/

#define NAMCONB1_HTOTAL		(288)	/* wrong */
#define NAMCONB1_HBSTART	(288)
#define NAMCONB1_VTOTAL		(262)	/* needs to be checked */
#define NAMCONB1_VBSTART	(224)

#define NAMCONB1_TILEMASKREGION		REGION_GFX1
#define NAMCONB1_TILEGFXREGION		REGION_GFX2
#define NAMCONB1_SPRITEGFXREGION	REGION_GFX3
#define NAMCONB1_ROTMASKREGION		REGION_GFX4
#define NAMCONB1_ROTGFXREGION		REGION_GFX5

#define NAMCONB1_TILEGFX		0
#define NAMCONB1_SPRITEGFX		1
#define NAMCONB1_ROTGFX			2

VIDEO_UPDATE( namconb1 );
VIDEO_START( namconb1 );

VIDEO_UPDATE( namconb2 );
VIDEO_START( namconb2 );
