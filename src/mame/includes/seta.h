/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#define	__uPD71054_TIMER	1

typedef struct _uPD71054_state uPD71054_state;
struct _uPD71054_state
{
	emu_timer *timer[3];			// Timer
	UINT16	max[3];				// Max counter
	UINT16	write_select;		// Max counter write select
	UINT8	reg[4];				//
};

typedef struct _game_offset game_offset;
struct _game_offset
{
	/* 2 values, for normal and flipped */
	const char *gamename;
	int sprite_offs[2];
	int tilemap_offs[2];
};

class seta_state : public driver_device
{
public:
	seta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_sharedram(*this,"sharedram"),
		m_workram(*this,"workram"),
		m_vregs(*this,"vregs"),
		m_vram_0(*this,"vram_0"),
		m_vctrl_0(*this,"vctrl_0"),
		m_vram_2(*this,"vram_2"),
		m_vctrl_2(*this,"vctrl_2"),
		m_paletteram(*this,"paletteram"),
		m_paletteram2(*this,"paletteram2"),
		m_kiwame_nvram(*this,"kiwame_nvram"),
		m_inttoote_key_select(*this,"inttoote_keysel"),
		m_inttoote_700000(*this,"inttoote_700000") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;

	optional_shared_ptr<UINT8> m_sharedram;
	optional_shared_ptr<UINT16> m_workram;
	optional_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_vram_0;
	optional_shared_ptr<UINT16> m_vctrl_0;
	optional_shared_ptr<UINT16> m_vram_2;
	optional_shared_ptr<UINT16> m_vctrl_2;
	optional_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_paletteram2;
	optional_shared_ptr<UINT16> m_kiwame_nvram;
	optional_shared_ptr<UINT16> m_inttoote_key_select;
	optional_shared_ptr<UINT16> m_inttoote_700000;

	int m_tiles_offset;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;	// Layer 0
	tilemap_t *m_tilemap_2;
	tilemap_t *m_tilemap_3;	// Layer 1
	int m_tilemaps_flip;
	int m_samples_bank;
	int m_color_mode_shift;
	int m_current_tilemap_mode[2];

	uPD71054_state m_uPD71054;
	const game_offset *m_global_offsets;

	bool m_coin_lockout_initialized;
	int m_coin_lockout;

	int m_sub_ctrl_data;

	int m_gun_input_bit;
	int m_gun_input_src;
	int m_gun_bit_count;
	int m_gun_old_clock;

	UINT8 m_usclssic_port_select;
	int m_keroppi_prize_hop;
	int m_keroppi_protection_count;

	int m_wiggie_soundlatch;

	UINT8 m_jockeyc_key_select;

	UINT8 m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	UINT16 m_magspeed_lights[3];

	UINT16 m_pairslove_protram[0x200];
	UINT16 m_pairslove_protram_old[0x200];
	UINT16 m_downtown_protection[0x200/2];

	DECLARE_WRITE16_MEMBER(seta_vregs_w);
	DECLARE_WRITE16_MEMBER(seta_vram_0_w);
	DECLARE_WRITE16_MEMBER(seta_vram_2_w);
	DECLARE_WRITE16_MEMBER(twineagl_tilebank_w);
	DECLARE_WRITE16_MEMBER(timer_regs_w);
	DECLARE_READ16_MEMBER(sharedram_68000_r);
	DECLARE_WRITE16_MEMBER(sharedram_68000_w);
	DECLARE_WRITE16_MEMBER(sub_ctrl_w);
	DECLARE_READ16_MEMBER(seta_dsw_r);
	DECLARE_READ16_MEMBER(calibr50_ip_r);
	DECLARE_WRITE16_MEMBER(calibr50_soundlatch_w);
	DECLARE_READ16_MEMBER(usclssic_dsw_r);
	DECLARE_READ16_MEMBER(usclssic_trackball_x_r);
	DECLARE_READ16_MEMBER(usclssic_trackball_y_r);
	DECLARE_WRITE16_MEMBER(usclssic_lockout_w);
	DECLARE_READ16_MEMBER(zombraid_gun_r);
	DECLARE_WRITE16_MEMBER(zombraid_gun_w);
	DECLARE_READ16_MEMBER(zingzipbl_unknown_r);
	DECLARE_READ16_MEMBER(keroppi_protection_r);
	DECLARE_READ16_MEMBER(keroppi_protection_init_r);
	DECLARE_READ16_MEMBER(keroppi_coin_r);
	DECLARE_WRITE16_MEMBER(keroppi_prize_w);
	DECLARE_WRITE16_MEMBER(setaroul_spriteylow_w);
	DECLARE_WRITE16_MEMBER(setaroul_spritectrl_w);
	DECLARE_WRITE16_MEMBER(setaroul_spritecode_w);
	DECLARE_READ16_MEMBER(setaroul_spritecode_r);
	DECLARE_READ16_MEMBER(krzybowl_input_r);
	DECLARE_WRITE16_MEMBER(msgundam_vregs_w);
	DECLARE_READ16_MEMBER(kiwame_nvram_r);
	DECLARE_WRITE16_MEMBER(kiwame_nvram_w);
	DECLARE_READ16_MEMBER(kiwame_input_r);
	DECLARE_READ16_MEMBER(thunderl_protection_r);
	DECLARE_WRITE16_MEMBER(thunderl_protection_w);
	DECLARE_READ8_MEMBER(wiggie_soundlatch_r);
	DECLARE_WRITE16_MEMBER(wiggie_soundlatch_w);
	DECLARE_WRITE16_MEMBER(utoukond_soundlatch_w);
	DECLARE_READ16_MEMBER(pairlove_prot_r);
	DECLARE_WRITE16_MEMBER(pairlove_prot_w);
	DECLARE_READ16_MEMBER(inttoote_dsw_r);
	DECLARE_READ16_MEMBER(inttoote_key_r);
	DECLARE_READ16_MEMBER(inttoote_700000_r);
	DECLARE_READ16_MEMBER(jockeyc_mux_r);
	DECLARE_WRITE16_MEMBER(jockeyc_mux_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_lockout_w);
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_READ8_MEMBER(downtown_ip_r);
	DECLARE_WRITE8_MEMBER(calibr50_soundlatch2_w);
	DECLARE_READ16_MEMBER(twineagl_debug_r);
	DECLARE_READ16_MEMBER(twineagl_200100_r);
	DECLARE_WRITE16_MEMBER(twineagl_200100_w);
	DECLARE_READ16_MEMBER(downtown_protection_r);
	DECLARE_WRITE16_MEMBER(downtown_protection_w);
	DECLARE_READ16_MEMBER(arbalest_debug_r);
	DECLARE_WRITE16_MEMBER(magspeed_lights_w);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_READ8_MEMBER(dsw2_r);
	DECLARE_DRIVER_INIT(downtown);
	DECLARE_DRIVER_INIT(rezon);
	DECLARE_DRIVER_INIT(twineagl);
	DECLARE_DRIVER_INIT(zombraid);
	DECLARE_DRIVER_INIT(crazyfgt);
	DECLARE_DRIVER_INIT(inttoote);
	DECLARE_DRIVER_INIT(metafox);
	DECLARE_DRIVER_INIT(arbalest);
	DECLARE_DRIVER_INIT(inttootea);
	DECLARE_DRIVER_INIT(wiggie);
	DECLARE_DRIVER_INIT(blandia);
	DECLARE_DRIVER_INIT(kiwame);
	DECLARE_DRIVER_INIT(eightfrc);
};

/*----------- defined in video/seta.c -----------*/

void seta_coin_lockout_w(running_machine &machine, int data);



PALETTE_INIT( blandia );
PALETTE_INIT( gundhara );
PALETTE_INIT( inttoote );
PALETTE_INIT( setaroul );
PALETTE_INIT( jjsquawk );
PALETTE_INIT( usclssic );
PALETTE_INIT( zingzip );

VIDEO_START( seta_no_layers);
VIDEO_START( twineagl_1_layer);
VIDEO_START( seta_1_layer);
VIDEO_START( seta_2_layers);
VIDEO_START( oisipuzl_2_layers );

SCREEN_UPDATE_IND16( seta );
SCREEN_UPDATE_IND16( seta_no_layers );
SCREEN_UPDATE_IND16( usclssic );
SCREEN_UPDATE_IND16( inttoote );
SCREEN_UPDATE_IND16( setaroul );

SCREEN_VBLANK( setaroul );

