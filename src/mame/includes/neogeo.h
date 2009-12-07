/*************************************************************************

    Neo-Geo hardware

*************************************************************************/


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

#define NEOGEO_BANK_AUDIO_CPU_CART_BANK		"audio_cart"
/* do not use 2, 3 and 4 */
#define NEOGEO_BANK_CARTRIDGE				"cartridge"
#define NEOGEO_BANK_BIOS					"bios"
#define NEOGEO_BANK_VECTORS					"vectors"
#define NEOGEO_BANK_AUDIO_CPU_MAIN_BANK		"audio_main"


/*----------- defined in drivers/neogeo.c -----------*/

void neogeo_set_display_position_interrupt_control(UINT16 data);
void neogeo_set_display_counter_msb(const address_space *space, UINT16 data);
void neogeo_set_display_counter_lsb(const address_space *space, UINT16 data);
void neogeo_acknowledge_interrupt(running_machine *machine, UINT16 data);
void neogeo_set_main_cpu_bank_address(const address_space *space, UINT32 bank_address);
READ16_HANDLER( neogeo_unmapped_r );


/*----------- defined in machine/neocrypt.c -----------*/

void kof99_neogeo_gfx_decrypt(running_machine *machine, int extra_xor);
void kof2000_neogeo_gfx_decrypt(running_machine *machine, int extra_xor);
void cmc42_neogeo_gfx_decrypt(running_machine *machine, int extra_xor);
void cmc50_neogeo_gfx_decrypt(running_machine *machine, int extra_xor);
void neogeo_cmc50_m1_decrypt(running_machine *machine);
void neo_pcm2_snk_1999(running_machine *machine, int value);
void neo_pcm2_swap(running_machine *machine, int value);
void neogeo_sfix_decrypt(running_machine *machine);
void kof99_decrypt_68k(running_machine *machine);
void garou_decrypt_68k(running_machine *machine);
void garouo_decrypt_68k(running_machine *machine);
void mslug3_decrypt_68k(running_machine *machine);
void kof2000_decrypt_68k(running_machine *machine);
void kof98_decrypt_68k(running_machine *machine);
void kof2002_decrypt_68k(running_machine *machine);
void matrim_decrypt_68k(running_machine *machine);
void mslug5_decrypt_68k(running_machine *machine);
void svc_px_decrypt(running_machine *machine);
void svcpcb_gfx_decrypt(running_machine *machine);
void svcpcb_s1data_decrypt(running_machine *machine);
void samsho5_decrypt_68k(running_machine *machine);
void kf2k3pcb_gfx_decrypt(running_machine *machine);
void kf2k3pcb_decrypt_68k(running_machine *machine);
void kf2k3pcb_decrypt_s1data(running_machine *machine);
void kof2003_decrypt_68k(running_machine *machine);
void kof2003h_decrypt_68k(running_machine *machine);
void kof2003biosdecode(running_machine *machine);
void samsh5sp_decrypt_68k(running_machine *machine);


/*----------- defined in machine/neoprot.c -----------*/

void neogeo_reset_rng(void);
void fatfury2_install_protection(running_machine *machine);
void mslugx_install_protection(running_machine *machine);
void kof99_install_protection(running_machine *machine);
void garou_install_protection(running_machine *machine);
void garouo_install_protection(running_machine *machine);
void mslug3_install_protection(running_machine *machine);
void kof2000_install_protection(running_machine *machine);
void install_kof98_protection(running_machine *machine);
void install_pvc_protection(running_machine *machine);


/*----------- defined in machine/neoboot.c -----------*/

void kog_px_decrypt(running_machine *machine);
void neogeo_bootleg_cx_decrypt(running_machine *machine);
void install_kof10th_protection(running_machine *machine);
void decrypt_kof10th(running_machine *machine);
void decrypt_kf10thep(running_machine *machine);
void decrypt_kf2k5uni(running_machine *machine);
void neogeo_bootleg_sx_decrypt(running_machine *machine, int value);
void kof2002b_gfx_decrypt(running_machine *machine, UINT8 *src, int size);
void kf2k2mp_decrypt(running_machine *machine);
void kf2k2mp2_px_decrypt(running_machine *machine);
void decrypt_cthd2003(running_machine *machine);
void patch_cthd2003(running_machine *machine);
void decrypt_ct2k3sp(running_machine *machine);
void decrypt_ct2k3sa(running_machine *machine);
void patch_ct2k3sa(running_machine *machine);
void decrypt_kof2k4se_68k(running_machine *machine);
void lans2004_decrypt_68k(running_machine *machine);
void lans2004_vx_decrypt(running_machine *machine);
void install_ms5plus_protection(running_machine *machine);
void svcboot_px_decrypt(running_machine *machine);
void svcboot_cx_decrypt(running_machine *machine);
void svcplus_px_decrypt(running_machine *machine);
void svcplus_px_hack(running_machine *machine);
void svcplusa_px_decrypt(running_machine *machine);
void svcsplus_px_decrypt(running_machine *machine);
void svcsplus_px_hack(running_machine *machine);
void kf2k3bl_px_decrypt(running_machine *machine);
void kf2k3bl_install_protection(running_machine *machine);
void kf2k3pl_px_decrypt(running_machine *machine);
void kf2k3upl_px_decrypt(running_machine *machine);
void kf2k3upl_install_protection(running_machine *machine);
void kf2k3pl_install_protection(running_machine *machine);
void samsho5b_px_decrypt(running_machine *machine);
void samsho5b_vx_decrypt(running_machine *machine);
void matrimbl_decrypt(running_machine *machine);


/*----------- defined in video/neogeo.c -----------*/

extern int neogeo_fixed_layer_bank_type;

VIDEO_START( neogeo );
VIDEO_RESET( neogeo );
VIDEO_UPDATE( neogeo );

READ16_HANDLER( neogeo_video_register_r );
WRITE16_HANDLER( neogeo_video_register_w );

void neogeo_set_palette_bank(running_machine *machine, UINT8 data);
void neogeo_set_screen_dark(running_machine *machine, UINT8 data);
READ16_HANDLER( neogeo_paletteram_r );
WRITE16_HANDLER( neogeo_paletteram_w );

void neogeo_set_fixed_layer_source(UINT8 data);

UINT8 neogeo_get_auto_animation_counter(void);
