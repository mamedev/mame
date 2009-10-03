/*----------- defined in drivers/namconb1.c -----------*/

extern UINT32 *namconb1_spritebank32;
extern UINT32 *namconb1_tilebank32;

/*----------- defined in video/namconb1.c -----------*/

#define NAMCONB1_HTOTAL		(288)	/* wrong */
#define NAMCONB1_HBSTART	(288)
#define NAMCONB1_VTOTAL		(262)	/* needs to be checked */
#define NAMCONB1_VBSTART	(224)

#define NAMCONB1_TILEMASKREGION		"tilemask"
#define NAMCONB1_TILEGFXREGION		"tile"
#define NAMCONB1_SPRITEGFXREGION	"sprite"
#define NAMCONB1_ROTMASKREGION		"rotmask"
#define NAMCONB1_ROTGFXREGION		"rot"

#define NAMCONB1_TILEGFX		0
#define NAMCONB1_SPRITEGFX		1
#define NAMCONB1_ROTGFX			2

VIDEO_UPDATE( namconb1 );
VIDEO_START( namconb1 );

VIDEO_UPDATE( namconb2 );
VIDEO_START( namconb2 );
