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

class seta_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, seta_state(machine)); }

	seta_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *sharedram;
	UINT16 *workram;
	UINT16 *vregs;
	UINT16 *vram_0;
	UINT16 *vctrl_0;
	UINT16 *vram_2;
	UINT16 *vctrl_2;
	UINT16 *spriteram;
	UINT16 *spriteram2;
	UINT16 *paletteram;
	size_t paletteram_size;

	int tiles_offset;
	tilemap_t *tilemap_0;
	tilemap_t *tilemap_1;	// Layer 0
	tilemap_t *tilemap_2;
	tilemap_t *tilemap_3;	// Layer 1
	int tilemaps_flip;
	int samples_bank;
	int taitox_banknum;

	uPD71054_state uPD71054;
	const game_offset *global_offsets;

	int coin_lockout;
	const game_driver *driver;

	int sub_ctrl_data;

	int gun_input_bit;
	int gun_input_src;
	int gun_bit_count;
	int gun_old_clock;

	UINT8 usclssic_port_select;
	int keroppi_prize_hop;
	int keroppi_protection_count;
	UINT16 *kiwame_nvram;

	int wiggie_soundlatch;

	UINT16 *inttoote_key_select;
	UINT16 *inttoote_700000;
	UINT8 jockeyc_key_select;

	UINT8 twineagl_xram[8];
	int twineagl_tilebank[4];

	UINT16 pairslove_protram[0x200];
	UINT16 pairslove_protram_old[0x200];
	UINT16 downtown_protection[0x200/2];
};

/*----------- defined in video/seta.c -----------*/

void seta_coin_lockout_w(running_machine *machine, int data);

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

VIDEO_UPDATE( seta );
VIDEO_UPDATE( seta_no_layers );
VIDEO_UPDATE( usclssic );
VIDEO_UPDATE( inttoote );


