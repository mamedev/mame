
class tmnt_state : public driver_device
{
public:
	tmnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *   m_tmnt2_1c0800;
	UINT16 *   m_sunset_104000;
	UINT16 *   m_tmnt2_rom;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
//  UINT8 *     m_nvram;    // currently cuebrick uses generic nvram handling
//  UINT8 *     m_cuebrick_nvram;
	UINT16 *m_spriteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];
	int        m_sorted_layer[3];	// this might not be necessary, but tmnt2 uses it in a strange way...

	tilemap_t  *m_roz_tilemap;
	int        m_glfgreat_roz_rom_bank;
	int        m_glfgreat_roz_char_bank;
	int        m_glfgreat_roz_rom_mode;
	int        m_glfgreat_pixel;
	int        m_prmrsocr_sprite_bank;
	int        m_blswhstl_rombank;
	int        m_tmnt_priorityflag;
	int        m_lastdim;
	int        m_lasten;
	int        m_dim_c;
	int        m_dim_v;	/* lgtnfght, ssriders, tmnt2 only */

	/* misc */
	int        m_tmnt_soundlatch;
	int        m_cuebrick_snd_irqlatch;
	int        m_cuebrick_nvram_bank;
	int        m_toggle;
	int        m_last;
	UINT16	   m_cuebrick_nvram[0x400 * 0x20];	// 32k paged in a 1k window

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k053260;
	device_t *m_k054539;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k053245;
	device_t *m_k053251;
	device_t *m_k053936;
	device_t *m_k054000;
	device_t *m_upd;
	device_t *m_samples;

	/* memory buffers */
	INT16      m_sampledata[0x40000];

	UINT8      m_irq5_mask;
};


/*----------- defined in video/tmnt.c -----------*/

extern void mia_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void cuebrick_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void tmnt_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void ssbl_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void blswhstl_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mia_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void tmnt_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void punkshot_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void thndrx2_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void lgtnfght_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void blswhstl_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void prmrsocr_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);

WRITE16_HANDLER( tmnt_paletteram_word_w );
WRITE16_HANDLER( tmnt_0a0000_w );
WRITE16_HANDLER( punkshot_0a0020_w );
WRITE16_HANDLER( lgtnfght_0a0018_w );
WRITE16_HANDLER( blswhstl_700300_w );
READ16_HANDLER( glfgreat_rom_r );
WRITE16_HANDLER( glfgreat_122000_w );
WRITE16_HANDLER( ssriders_eeprom_w );
WRITE16_HANDLER( ssriders_1c0300_w );
WRITE16_HANDLER( prmrsocr_122000_w );
WRITE16_HANDLER( tmnt_priority_w );
READ16_HANDLER( glfgreat_ball_r );
READ16_HANDLER( prmrsocr_rom_r );

VIDEO_START( cuebrick );
VIDEO_START( mia );
VIDEO_START( tmnt );
VIDEO_START( lgtnfght );
VIDEO_START( blswhstl );
VIDEO_START( glfgreat );
VIDEO_START( prmrsocr );

SCREEN_UPDATE( mia );
SCREEN_UPDATE( tmnt );
SCREEN_UPDATE( punkshot );
SCREEN_UPDATE( lgtnfght );
SCREEN_UPDATE( glfgreat );
SCREEN_UPDATE( tmnt2 );
SCREEN_UPDATE( thndrx2 );

SCREEN_EOF( blswhstl );
