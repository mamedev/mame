/*************************************************************************

    Neo-Geo hardware

*************************************************************************/


#define VERBOSE 	(0)

#define NEOGEO_MASTER_CLOCK					(24000000)
#define NEOGEO_MAIN_CPU_CLOCK				(NEOGEO_MASTER_CLOCK / 2)
#define NEOGEO_AUDIO_CPU_CLOCK				(NEOGEO_MASTER_CLOCK / 6)
#define NEOGEO_YM2610_CLOCK					(NEOGEO_MASTER_CLOCK / 3)
#define NEOGEO_PIXEL_CLOCK					(NEOGEO_MASTER_CLOCK / 4)
#define NEOGEO_HTOTAL						(0x180)
#define NEOGEO_HBEND						(0x01e)	/* this should really be 29.5 */
#define NEOGEO_HBSTART						(0x15e) /* this should really be 349.5 */
#define NEOGEO_VTOTAL						(0x108)
#define NEOGEO_VBEND						(0x010)
#define NEOGEO_VBSTART						(0x0f0)
#define NEOGEO_VSSTART						(0x000)
#define NEOGEO_VBLANK_RELOAD_HPOS			(0x11f)

#define NEOGEO_REGION_MAIN_CPU_BIOS			(REGION_USER1)
#define NEOGEO_REGION_MAIN_CPU_CARTRIDGE	(REGION_CPU1)
#define NEOGEO_REGION_AUDIO_CPU_BIOS		(REGION_USER2)
#define NEOGEO_REGION_AUDIO_CPU_CARTRIDGE	(REGION_CPU2)
#define NEOGEO_REGION_AUDIO_CPU_ENCRYPTED	(REGION_USER3)
#define NEOGEO_REGION_FIXED_LAYER_BIOS		(REGION_GFX2)
#define NEOGEO_REGION_FIXED_LAYER_CARTRIDGE	(REGION_GFX1)
#define NEOGEO_REGION_SPRITES				(REGION_GFX3)
#define NEOGEO_REGION_AUDIO_DATA_1			(REGION_SOUND1)
#define NEOGEO_REGION_AUDIO_DATA_2			(REGION_SOUND2)
#define NEOGEO_REGION_ZOOM_Y_TABLE			(REGION_GFX4)

#define NEOGEO_BANK_AUDIO_CPU_CART_BANK		(1)
/* do not use 2, 3 and 4 */
#define NEOGEO_BANK_CARTRIDGE				(5)
#define NEOGEO_BANK_BIOS					(6)
#define NEOGEO_BANK_VECTORS					(7)
#define NEOGEO_BANK_EXTRA_RAM				(8)
#define NEOGEO_BANK_AUDIO_CPU_MAIN_BANK		(9)


/*----------- defined in drivers/neogeo.c -----------*/

void neogeo_set_display_counter_msb(UINT16 data);
void neogeo_set_display_counter_lsb(UINT16 data);
void neogeo_acknowledge_interrupt(UINT16 data);
void neogeo_set_main_cpu_bank_address(UINT32 bank_address);
READ16_HANDLER( neogeo_unmapped_r );


/*----------- defined in machine/neocrypt.c -----------*/

void kof99_neogeo_gfx_decrypt(int extra_xor);
void kof2000_neogeo_gfx_decrypt(int extra_xor);
void cmc50_neogeo_gfx_decrypt(int extra_xor);
void cmc42_neogeo_gfx_decrypt(int extra_xor);
void kof99_decrypt_68k(void);
void garou_decrypt_68k(void);
void garouo_decrypt_68k(void);
void mslug3_decrypt_68k(void);
void kof2000_decrypt_68k(void);
void kof98_decrypt_68k(void);
void kof2002_decrypt_68k(void);
void matrim_decrypt_68k(void);
void mslug5_decrypt_68k(void);
void svcchaos_px_decrypt(void);
void svcpcb_gfx_decrypt(void);
void svcpcb_s1data_decrypt(void);
void samsho5_decrypt_68k(void);
void kf2k3pcb_gfx_decrypt(void);
void kf2k3pcb_decrypt_68k(void);
void kf2k3pcb_decrypt_s1data(void);
void kof2003_decrypt_68k(void);
void kof2003biosdecode(void);
void samsh5p_decrypt_68k(void);

void neo_pcm2_snk_1999(int value);
void neo_pcm2_swap(int value);


/*----------- defined in machine/neoprot.c -----------*/

void neogeo_reset_rng(void);
void fatfury2_install_protection(void);
void mslugx_install_protection(void);
void kof99_install_protection(void);
void garou_install_protection(void);
void garouo_install_protection(void);
void mslug3_install_protection(void);
void kof2000_install_protection(void);
void install_kof98_protection(void);
void install_pvc_protection(void);


/*----------- defined in machine/neoboot.c -----------*/

void kog_px_decrypt(void);
void neogeo_bootleg_cx_decrypt(void);
void install_kof10th_protection(void);
void decrypt_kof10th(void);
void decrypt_kf10thep(void);
void decrypt_kf2k5uni(void);
void neogeo_bootleg_sx_decrypt(int value);
void kf2k2mp_decrypt(void);
void kof2km2_px_decrypt(void);
void decrypt_cthd2003(void);
void patch_cthd2003(void);
void decrypt_ct2k3sp(void);
void decrypt_ct2k3sa(void);
void patch_ct2k3sa(void);
void decrypt_kof2k4se_68k(void);
void lans2004_decrypt_68k(void);
void lans2004_vx_decrypt(void);
void install_ms5plus_protection(void);
void svcboot_px_decrypt( void );
void svcboot_cx_decrypt( void );
void svcplus_px_decrypt( void );
void svcplus_px_hack( void );
void svcplusa_px_decrypt( void );
void svcsplus_px_decrypt( void );
void svcsplus_px_hack( void );
void kof2003b_px_decrypt( void );
void kof2003b_install_protection(void);
void kof2k3pl_px_decrypt( void );
void kof2k3up_px_decrypt( void );
void kof2k3up_install_protection(void);
void kf2k3pl_install_protection(void);
void samsh5bl_px_decrypt( void );


/*----------- defined in video/neogeo.c -----------*/

extern int neogeo_fixed_layer_bank_type;

VIDEO_START( neogeo );
VIDEO_RESET( neogeo );
VIDEO_UPDATE( neogeo );

READ16_HANDLER( neogeo_video_register_r );
WRITE16_HANDLER( neogeo_video_register_w );

void neogeo_set_display_poisition_interrupt_control(UINT16 data);

void neogeo_set_palette_bank(UINT8 data);
void neogeo_set_screen_dark(UINT8 data);
READ16_HANDLER( neogeo_paletteram_r );
WRITE16_HANDLER( neogeo_paletteram_w );

void neogeo_set_fixed_layer_source(UINT8 data);

UINT8 neogeo_get_auto_animation_counter(void);
