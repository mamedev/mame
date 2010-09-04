
class tmnt_state : public driver_device
{
public:
	tmnt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	INT16 *    sampledata;
	UINT16 *   tmnt2_1c0800;
	UINT16 *   sunset_104000;
	UINT16 *   tmnt2_rom;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
//  UINT8 *     nvram;    // currently cuebrick uses generic nvram handling
//  UINT8 *     cuebrick_nvram;

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];
	int        sorted_layer[3];	// this might not be necessary, but tmnt2 uses it in a strange way...

	tilemap_t  *roz_tilemap;
	int        glfgreat_roz_rom_bank, glfgreat_roz_char_bank, glfgreat_roz_rom_mode;
	int        glfgreat_pixel;
	int        prmrsocr_sprite_bank;
	int        blswhstl_rombank;
	int        tmnt_priorityflag;
	int        lastdim, lasten, dim_c, dim_v;	/* lgtnfght, ssriders, tmnt2 only */

	/* misc */
	int        tmnt_soundlatch;
	int        cuebrick_snd_irqlatch, cuebrick_nvram_bank;
	int        toggle, last;
	UINT16	   m_cuebrick_nvram[0x400 * 0x20];	// 32k paged in a 1k window

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k007232;
	running_device *k053260;
	running_device *k054539;
	running_device *k052109;
	running_device *k051960;
	running_device *k053245;
	running_device *k053251;
	running_device *k053936;
	running_device *k054000;
	running_device *upd;
	running_device *samples;
};


/*----------- defined in video/tmnt.c -----------*/

extern void mia_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void cuebrick_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void tmnt_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void ssbl_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void blswhstl_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mia_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void tmnt_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void punkshot_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void thndrx2_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void lgtnfght_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void blswhstl_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void prmrsocr_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

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

VIDEO_UPDATE( mia );
VIDEO_UPDATE( tmnt );
VIDEO_UPDATE( punkshot );
VIDEO_UPDATE( lgtnfght );
VIDEO_UPDATE( glfgreat );
VIDEO_UPDATE( tmnt2 );
VIDEO_UPDATE( thndrx2 );

VIDEO_EOF( blswhstl );
