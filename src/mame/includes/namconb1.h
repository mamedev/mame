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
	namconb1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu")
		{ }

	UINT32 *m_nvmem32;
	UINT16 *m_namconb_shareram;
	UINT8 m_namconb_cpureg[32];
	int m_vblank_irq_active;
	int m_pos_irq_active;
	UINT16 m_count;
	UINT8 m_nbx_port6;
	UINT32 *m_spritebank32;
	UINT32 *m_tilebank32;
	UINT32 m_tilemap_tile_bank[4];

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
};


/*----------- defined in video/namconb1.c -----------*/

SCREEN_UPDATE( namconb1 );
VIDEO_START( namconb1 );

SCREEN_UPDATE( namconb2 );
VIDEO_START( namconb2 );
