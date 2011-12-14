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
		m_subcpu(*this,"sub")
		{ }

	UINT8 *m_sharedram;
	UINT16 *m_workram;
	UINT16 *m_vregs;
	UINT16 *m_vram_0;
	UINT16 *m_vctrl_0;
	UINT16 *m_vram_2;
	UINT16 *m_vctrl_2;
	UINT16 *m_paletteram;
	size_t m_paletteram_size;
	UINT16 *m_paletteram2;
	size_t m_paletteram2_size;

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
	UINT16 *m_kiwame_nvram;

	int m_wiggie_soundlatch;

	UINT16 *m_inttoote_key_select;
	UINT16 *m_inttoote_700000;
	UINT8 m_jockeyc_key_select;

	UINT8 m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	UINT16 m_pairslove_protram[0x200];
	UINT16 m_pairslove_protram_old[0x200];
	UINT16 m_downtown_protection[0x200/2];

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
};

/*----------- defined in video/seta.c -----------*/

void seta_coin_lockout_w(running_machine &machine, int data);

WRITE16_HANDLER( twineagl_tilebank_w );

WRITE16_HANDLER( seta_vram_0_w );
WRITE16_HANDLER( seta_vram_2_w );
WRITE16_HANDLER( seta_vregs_w );

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

SCREEN_UPDATE( seta );
SCREEN_UPDATE( seta_no_layers );
SCREEN_UPDATE( usclssic );
SCREEN_UPDATE( inttoote );
SCREEN_UPDATE( setaroul );

SCREEN_EOF( setaroul );

