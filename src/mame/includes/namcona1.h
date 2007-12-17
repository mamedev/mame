/*----------- defined in drivers/namcona1.c -----------*/

extern int namcona1_gametype;


/*----------- defined in video/namcona1.c -----------*/

enum
{
	NAMCO_CGANGPZL,
	NAMCO_EMERALDA,
	NAMCO_KNCKHEAD,
	NAMCO_BKRTMAQ,
	NAMCO_EXBANIA,
	NAMCO_QUIZTOU,
	NAMCO_SWCOURT,
	NAMCO_TINKLPIT,
	NAMCO_NUMANATH,
	NAMCO_FA,
	NAMCO_XDAY2
};

#define NA1_NVRAM_SIZE (0x800)

extern UINT16 *namcona1_workram;
extern UINT16 *namcona1_vreg;
extern UINT16 *namcona1_scroll;
extern UINT16 *namcona1_sparevram;

extern WRITE16_HANDLER( namcona1_videoram_w );
extern READ16_HANDLER( namcona1_videoram_r );

extern READ16_HANDLER( namcona1_gfxram_r );
extern WRITE16_HANDLER( namcona1_gfxram_w );

extern READ16_HANDLER( namcona1_paletteram_r );
extern WRITE16_HANDLER( namcona1_paletteram_w );

extern VIDEO_UPDATE( namcona1 );
extern VIDEO_START( namcona1 );
