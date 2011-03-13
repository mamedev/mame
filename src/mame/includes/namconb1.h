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

class namconb1_state : public driver_device
{
public:
	namconb1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *nvmem32;
	UINT16 *namconb_shareram;
	UINT8 namconb_cpureg[32];
	int vblank_irq_active;
	int pos_irq_active;
	UINT16 count;
	UINT8 nbx_port6;
	UINT32 *spritebank32;
	UINT32 *tilebank32;
	UINT32 tilemap_tile_bank[4];
};


/*----------- defined in video/namconb1.c -----------*/

SCREEN_UPDATE( namconb1 );
VIDEO_START( namconb1 );

SCREEN_UPDATE( namconb2 );
VIDEO_START( namconb2 );
