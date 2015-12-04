// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

  namcos2.h

  Common functions & declarations for the Namco System 2 driver

***************************************************************************/

#include "namcoic.h"
#include "cpu/m6502/m3745x.h"
#include "video/c45.h"

/* CPU reference numbers */

#define CPU_MASTER  0
#define CPU_SLAVE   1
#define CPU_SOUND   2
#define CPU_MCU     3
#define CPU_GPU     5


/*********************************************/
/* IF GAME SPECIFIC HACKS ARE REQUIRED THEN  */
/* USE THE m_gametype MEMBER TO FIND */
/* OUT WHAT GAME IS RUNNING                  */
/*********************************************/

enum
{
	/* Namco System 2 */
	NAMCOS2_ASSAULT = 0x1000,
	NAMCOS2_ASSAULT_JP,
	NAMCOS2_ASSAULT_PLUS,
	NAMCOS2_BUBBLE_TROUBLE,
	NAMCOS2_BURNING_FORCE,
	NAMCOS2_COSMO_GANG,
	NAMCOS2_COSMO_GANG_US,
	NAMCOS2_DIRT_FOX,
	NAMCOS2_DIRT_FOX_JP,
	NAMCOS2_DRAGON_SABER,
	NAMCOS2_FINAL_LAP,
	NAMCOS2_FINAL_LAP_2,
	NAMCOS2_FINAL_LAP_3,
	NAMCOS2_FINEST_HOUR,
	NAMCOS2_FOUR_TRAX,
	NAMCOS2_GOLLY_GHOST,
	NAMCOS2_LUCKY_AND_WILD,
	NAMCOS2_MARVEL_LAND,
	NAMCOS2_METAL_HAWK,
	NAMCOS2_MIRAI_NINJA,
	NAMCOS2_ORDYNE,
	NAMCOS2_PHELIOS,
	NAMCOS2_ROLLING_THUNDER_2,
	NAMCOS2_STEEL_GUNNER,
	NAMCOS2_STEEL_GUNNER_2,
	NAMCOS2_SUPER_WSTADIUM,
	NAMCOS2_SUPER_WSTADIUM_92,
	NAMCOS2_SUPER_WSTADIUM_92T,
	NAMCOS2_SUPER_WSTADIUM_93,
	NAMCOS2_SUZUKA_8_HOURS,
	NAMCOS2_SUZUKA_8_HOURS_2,
	NAMCOS2_VALKYRIE,
	NAMCOS2_KYUUKAI_DOUCHUUKI,

	/* Namco System21 */
	NAMCOS21_AIRCOMBAT,
	NAMCOS21_STARBLADE,
	NAMCOS21_CYBERSLED,
	NAMCOS21_SOLVALOU,
	NAMCOS21_WINRUN91,
	NAMCOS21_DRIVERS_EYES,

	/* Namco NB1 */
	NAMCONB1_NEBULRAY,
	NAMCONB1_GUNBULET,
	NAMCONB1_GSLGR94U,
	NAMCONB1_GSLGR94J,
	NAMCONB1_SWS95,
	NAMCONB1_SWS96,
	NAMCONB1_SWS97,
	NAMCONB1_VSHOOT,

	/* Namco NB2 */
	NAMCONB2_OUTFOXIES,
	NAMCONB2_MACH_BREAKERS,

	/* Namco System FL */
	NAMCOFL_SPEED_RACER,
	NAMCOFL_FINAL_LAP_R
};


// fix me -- most of this should be devices eventually
class namcos2_shared_state : public driver_device
{
public:
	namcos2_shared_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_dspmaster(*this, "dspmaster"),
			m_dspslave(*this, "dspslave"),
			m_c68(*this, "c68"),
			m_gpu(*this, "gpu"),
			m_gametype(0),
			m_c169_roz_videoram(*this, "rozvideoram", 0),
			m_c169_roz_gfxbank(0),
			m_c169_roz_mask(nullptr),
			m_c355_obj_gfxbank(0),
			m_c355_obj_palxor(0),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_slave(*this, "slave"),
			m_mcu(*this, "mcu"),
			m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	optional_device<cpu_device> m_dspmaster;
	optional_device<cpu_device> m_dspslave;
	optional_device<m37450_device> m_c68;
	optional_device<cpu_device> m_gpu; //to be moved to namco21_state after disentangling

	// game type helpers
	bool is_system21();
	int m_gametype;

	emu_timer *m_posirq_timer;
	int m_mcu_analog_ctrl;
	int m_mcu_analog_data;
	int m_mcu_analog_complete;
	UINT8 *m_eeprom;
	UINT16  m_68k_master_C148[0x20];
	UINT16  m_68k_slave_C148[0x20];
	UINT16  m_68k_gpu_C148[0x20];

	// C123 Tilemap Emulation
	// TODO: merge with namcos1.c implementation and convert to device
public:
	DECLARE_WRITE16_MEMBER( c123_tilemap_videoram_w );
	DECLARE_READ16_MEMBER( c123_tilemap_videoram_r );
	DECLARE_WRITE16_MEMBER( c123_tilemap_control_w );
	DECLARE_READ16_MEMBER( c123_tilemap_control_r );
	TILE_GET_INFO_MEMBER( get_tile_info0 );
	TILE_GET_INFO_MEMBER( get_tile_info1 );
	TILE_GET_INFO_MEMBER( get_tile_info2 );
	TILE_GET_INFO_MEMBER( get_tile_info3 );
	TILE_GET_INFO_MEMBER( get_tile_info4 );
	TILE_GET_INFO_MEMBER( get_tile_info5 );
	void namco_tilemap_init(int gfxbank, void *pMaskROM, void (*cb)( running_machine &machine, UINT16 code, int *gfx, int *mask) );

	// C169 ROZ Layer Emulation
public:
	void c169_roz_init(int gfxbank, const char *maskregion);
	void c169_roz_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	DECLARE_READ16_MEMBER( c169_roz_control_r );
	DECLARE_WRITE16_MEMBER( c169_roz_control_w );
	DECLARE_READ16_MEMBER( c169_roz_bank_r );
	DECLARE_WRITE16_MEMBER( c169_roz_bank_w );
	DECLARE_READ16_MEMBER( c169_roz_videoram_r );
	DECLARE_WRITE16_MEMBER( c169_roz_videoram_w );

protected:
	struct roz_parameters
	{
		UINT32 left, top, size;
		UINT32 startx, starty;
		int incxx, incxy, incyx, incyy;
		int color, priority;
	};
	void c169_roz_unpack_params(const UINT16 *source, roz_parameters &params);
	void c169_roz_draw_helper(screen_device &screen, bitmap_ind16 &bitmap, tilemap_t &tmap, const rectangle &clip, const roz_parameters &params);
	void c169_roz_draw_scanline(screen_device &screen, bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect);
	void c169_roz_get_info(tile_data &tileinfo, int tile_index, int which);
	TILE_GET_INFO_MEMBER( c169_roz_get_info0 );
	TILE_GET_INFO_MEMBER( c169_roz_get_info1 );
	TILEMAP_MAPPER_MEMBER( c169_roz_mapper );

	static const int ROZ_TILEMAP_COUNT = 2;
	tilemap_t *m_c169_roz_tilemap[ROZ_TILEMAP_COUNT];
	UINT16 m_c169_roz_bank[0x10/2];
	UINT16 m_c169_roz_control[0x20/2];
	optional_shared_ptr<UINT16> m_c169_roz_videoram;
	int m_c169_roz_gfxbank;
	UINT8 *m_c169_roz_mask;

	// C355 Motion Object Emulation
public:
	typedef delegate<int (int)> c355_obj_code2tile_delegate;
	// for pal_xor, supply either 0x0 (normal) or 0xf (palette mapping reversed)
	void c355_obj_init(int gfxbank, int pal_xor, c355_obj_code2tile_delegate code2tile);
	int c355_obj_default_code2tile(int code);
	void c355_obj_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void c355_obj_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);
	DECLARE_READ16_MEMBER( c355_obj_ram_r );
	DECLARE_WRITE16_MEMBER( c355_obj_ram_w );
	DECLARE_READ16_MEMBER( c355_obj_position_r );
	DECLARE_WRITE16_MEMBER( c355_obj_position_w );
	DECLARE_MACHINE_START(namcos2);
	DECLARE_MACHINE_RESET(namcos2);
protected:
	// C355 Motion Object internals
	template<class _BitmapClass>
	void c355_obj_draw_sprite(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, const UINT16 *pSource, int pri, int zpos);
	template<class _BitmapClass>
	void c355_obj_draw_list(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, const UINT16 *pSpriteList16, const UINT16 *pSpriteTable);

	c355_obj_code2tile_delegate m_c355_obj_code2tile;
	int m_c355_obj_gfxbank;
	int m_c355_obj_palxor;
	UINT16 m_c355_obj_position[4];
	UINT16 m_c355_obj_ram[0x20000/2];

	UINT8 m_player_mux;
	inline void namcoic_get_tile_info(tile_data &tileinfo,int tile_index,UINT16 *vram);

public:
	// general
	void zdrawgfxzoom(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);
	void zdrawgfxzoom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int zpos);
	INTERRUPT_GEN_MEMBER(namcos2_68k_master_vblank);
	INTERRUPT_GEN_MEMBER(namcos2_68k_slave_vblank);
	INTERRUPT_GEN_MEMBER(namcos2_68k_gpu_vblank);
	TIMER_CALLBACK_MEMBER(namcos2_posirq_tick);
	void adjust_posirq_timer( int scanline );
	void init_c148();
	void reset_all_subcpus(int state);
	UINT16 readwrite_c148( address_space &space, offs_t offset, UINT16 data, int bWrite );
	int get_posirq_scanline();

	DECLARE_WRITE8_MEMBER( namcos2_68k_eeprom_w );
	DECLARE_READ8_MEMBER( namcos2_68k_eeprom_r );
	DECLARE_WRITE16_MEMBER( namcos2_68k_master_C148_w );
	DECLARE_READ16_MEMBER( namcos2_68k_master_C148_r );

	DECLARE_WRITE16_MEMBER( namcos2_68k_slave_C148_w );
	DECLARE_READ16_MEMBER( namcos2_68k_slave_C148_r );

	DECLARE_WRITE8_MEMBER( namcos2_mcu_port_d_w );
	DECLARE_READ8_MEMBER( namcos2_mcu_port_d_r );
	DECLARE_WRITE8_MEMBER( namcos2_mcu_analog_ctrl_w );
	DECLARE_READ8_MEMBER( namcos2_mcu_analog_ctrl_r );
	DECLARE_WRITE8_MEMBER( namcos2_mcu_analog_port_w );
	DECLARE_READ8_MEMBER( namcos2_mcu_analog_port_r );
	DECLARE_WRITE8_MEMBER( namcos2_sound_bankselect_w );

	/* TODO: this should belong to namcos21_state */
	DECLARE_WRITE16_MEMBER( namcos21_68k_gpu_C148_w );
	DECLARE_READ16_MEMBER( namcos21_68k_gpu_C148_r );
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_slave;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

class namcos2_state : public namcos2_shared_state
{
public:
	namcos2_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos2_shared_state(mconfig, type, tag),
			m_dpram(*this, "dpram"),
			m_paletteram(*this, "paletteram"),
			m_spriteram(*this, "spriteram"),
			m_serial_comms_ram(*this, "serialram"),
			m_rozram(*this, "rozram"),
			m_roz_ctrl(*this, "rozctrl"),
			m_c45_road(*this, "c45_road")
	{ }

	DECLARE_READ8_MEMBER(c68_p5_r);
	DECLARE_WRITE8_MEMBER(c68_p3_w);
	DECLARE_READ16_MEMBER(dpram_word_r);
	DECLARE_WRITE16_MEMBER(dpram_word_w);
	DECLARE_READ8_MEMBER(dpram_byte_r);
	DECLARE_WRITE8_MEMBER(dpram_byte_w);
	DECLARE_READ8_MEMBER(ack_mcu_vbl_r);
	DECLARE_DRIVER_INIT(cosmogng);
	DECLARE_DRIVER_INIT(sgunner2);
	DECLARE_DRIVER_INIT(kyukaidk);
	DECLARE_DRIVER_INIT(bubbletr);
	DECLARE_DRIVER_INIT(suzuk8h2);
	DECLARE_DRIVER_INIT(burnforc);
	DECLARE_DRIVER_INIT(gollygho);
	DECLARE_DRIVER_INIT(rthun2j);
	DECLARE_DRIVER_INIT(sws);
	DECLARE_DRIVER_INIT(finehour);
	DECLARE_DRIVER_INIT(finallap);
	DECLARE_DRIVER_INIT(dirtfoxj);
	DECLARE_DRIVER_INIT(marvlanj);
	DECLARE_DRIVER_INIT(sws92);
	DECLARE_DRIVER_INIT(dsaber);
	DECLARE_DRIVER_INIT(assault);
	DECLARE_DRIVER_INIT(mirninja);
	DECLARE_DRIVER_INIT(finalap2);
	DECLARE_DRIVER_INIT(valkyrie);
	DECLARE_DRIVER_INIT(fourtrax);
	DECLARE_DRIVER_INIT(finalap3);
	DECLARE_DRIVER_INIT(luckywld);
	DECLARE_DRIVER_INIT(assaultj);
	DECLARE_DRIVER_INIT(dsaberj);
	DECLARE_DRIVER_INIT(suzuka8h);
	DECLARE_DRIVER_INIT(phelios);
	DECLARE_DRIVER_INIT(sws93);
	DECLARE_DRIVER_INIT(metlhawk);
	DECLARE_DRIVER_INIT(sws92g);
	DECLARE_DRIVER_INIT(assaultp_hack);
	DECLARE_DRIVER_INIT(assaultp);
	DECLARE_DRIVER_INIT(ordyne);
	DECLARE_DRIVER_INIT(marvland);
	DECLARE_DRIVER_INIT(rthun2);

	virtual void video_start();
	void video_start_finallap();
	void video_start_luckywld();
	void video_start_metlhawk();
	void video_start_sgunner();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_finallap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_luckywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_metlhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sgunner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER( roz_tile_info );

	DECLARE_READ16_MEMBER( paletteram_word_r );
	DECLARE_WRITE16_MEMBER( paletteram_word_w );
	DECLARE_WRITE16_MEMBER( rozram_word_w );
	DECLARE_READ16_MEMBER( gfx_ctrl_r );
	DECLARE_WRITE16_MEMBER( gfx_ctrl_w );
	DECLARE_READ16_MEMBER( serial_comms_ram_r );
	DECLARE_WRITE16_MEMBER( serial_comms_ram_w );
	DECLARE_READ16_MEMBER( serial_comms_ctrl_r );
	DECLARE_WRITE16_MEMBER( serial_comms_ctrl_w );

	void draw_sprite_init();
	void update_palette();
	void apply_clip( rectangle &clip, const rectangle &cliprect );
	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control );
	void draw_sprites_metalhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	UINT16 get_palette_register( int which );

	int get_pos_irq_scanline() { return (get_palette_register(5) - 32) & 0xff; }

	required_shared_ptr<UINT8> m_dpram; /* 2Kx8 */
	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_serial_comms_ram;
	optional_shared_ptr<UINT16> m_rozram;
	optional_shared_ptr<UINT16> m_roz_ctrl;
	tilemap_t *m_tilemap_roz;
	UINT16 m_gfx_ctrl;
	UINT16 m_serial_comms_ctrl[0x8];
	unsigned m_finallap_prot_count;
	int m_sendval;

	optional_device<namco_c45_road_device> m_c45_road;

	DECLARE_READ16_MEMBER( namcos2_68k_key_r );
	DECLARE_WRITE16_MEMBER( namcos2_68k_key_w );
	DECLARE_READ16_MEMBER( namcos2_finallap_prot_r );

};

/*----------- defined in video/namcos2.c -----------*/

#define NAMCOS21_NUM_COLORS 0x8000

/**************************************************************/
/*  ROZ - Rotate & Zoom memory function handlers              */
/**************************************************************/

/*----------- defined in machine/namcos2.c -----------*/

extern void (*namcos2_kickstart)(running_machine &machine, int internal);


/**************************************************************/
/* Non-shared memory custom IO device - IRQ/Inputs/Outputs   */
/**************************************************************/

#define NAMCOS2_C148_0          0       /* 0x1c0000 */
#define NAMCOS2_C148_1          1       /* 0x1c2000 */
#define NAMCOS2_C148_2          2       /* 0x1c4000 */
#define NAMCOS2_C148_CPUIRQ     3       /* 0x1c6000 */
#define NAMCOS2_C148_EXIRQ      4       /* 0x1c8000 */
#define NAMCOS2_C148_POSIRQ     5       /* 0x1ca000 */
#define NAMCOS2_C148_SERIRQ     6       /* 0x1cc000 */
#define NAMCOS2_C148_VBLANKIRQ  7       /* 0x1ce000 */

/**************************************************************/
/* MASTER CPU RAM MEMORY                                      */
/**************************************************************/

#define NAMCOS2_68K_MASTER_RAM  "bank3"

/**************************************************************/
/* SLAVE CPU RAM MEMORY                                       */
/**************************************************************/

#define NAMCOS2_68K_SLAVE_RAM   "bank4"

/**************************************************************/
/*                                                            */
/**************************************************************/
#define BANKED_SOUND_ROM        "bank6"

/**************************************************************/
/* Sound CPU support handlers - 6809                          */
/**************************************************************/
