/*************************************************************************

    Neo-Geo hardware

*************************************************************************/


#define NEOGEO_MASTER_CLOCK                     (24000000)
#define NEOGEO_MAIN_CPU_CLOCK                   (NEOGEO_MASTER_CLOCK / 2)
#define NEOGEO_AUDIO_CPU_CLOCK                  (NEOGEO_MASTER_CLOCK / 6)
#define NEOGEO_YM2610_CLOCK                     (NEOGEO_MASTER_CLOCK / 3)
#define NEOGEO_PIXEL_CLOCK                      (NEOGEO_MASTER_CLOCK / 4)
#define NEOGEO_HTOTAL                           (0x180)
#define NEOGEO_HBEND                            (0x01e)	/* this should really be 29.5 */
#define NEOGEO_HBSTART                          (0x15e)	/* this should really be 349.5 */
#define NEOGEO_VTOTAL                           (0x108)
#define NEOGEO_VBEND                            (0x010)
#define NEOGEO_VBSTART                          (0x0f0)
#define NEOGEO_VSSTART                          (0x000)
#define NEOGEO_VBLANK_RELOAD_HPOS               (0x11f)

#define NEOGEO_BANK_AUDIO_CPU_CART_BANK         "audio_cart"
/* do not use 2, 3 and 4 */
#define NEOGEO_BANK_CARTRIDGE                   "cartridge"
#define NEOGEO_BANK_BIOS                        "bios"
#define NEOGEO_BANK_VECTORS                     "vectors"
#define NEOGEO_BANK_AUDIO_CPU_MAIN_BANK         "audio_main"



typedef struct _neogeo_state neogeo_state;
struct _neogeo_state
{
	/* memory pointers */
//  UINT8      *memcard_data;   // this currently uses generic handlers
//  UINT16     *save_ram;       // this currently uses generic handlers

	/* video-related */
	UINT8      *sprite_gfx;
	UINT32     sprite_gfx_address_mask;
	UINT16     *videoram;
	UINT16     *palettes[2]; /* 0x100*16 2 byte palette entries */
	pen_t      *pens;
	UINT8      palette_bank;
	UINT8      screen_dark;
	UINT16     videoram_read_buffer;
	UINT16     videoram_modulo;
	UINT16     videoram_offset;

	UINT8      fixed_layer_source;

	UINT8      auto_animation_speed;
	UINT8      auto_animation_disabled;
	UINT8      auto_animation_counter;
	UINT8      auto_animation_frame_counter;

	const UINT8 *region_zoomy;


	/* palette */
	double     rgb_weights_normal[5];
	double     rgb_weights_normal_bit15[5];
	double     rgb_weights_dark[5];
	double     rgb_weights_dark_bit15[5];

	/* timers */
	emu_timer  *display_position_interrupt_timer;
	emu_timer  *display_position_vblank_timer;
	emu_timer  *vblank_interrupt_timer;
	emu_timer  *auto_animation_timer;
	emu_timer  *sprite_line_timer;
	UINT8      display_position_interrupt_control;
	UINT32     display_counter;
	UINT32     vblank_interrupt_pending;
	UINT32     display_position_interrupt_pending;
	UINT32     irq3_pending;

	/* misc */
	UINT8      controller_select;

	UINT32     main_cpu_bank_address;
	UINT8      main_cpu_vector_table_source;

	UINT8      audio_result;
	UINT8      audio_cpu_banks[4];
	UINT8      audio_cpu_rom_source;
	UINT8      audio_cpu_rom_source_last;

	UINT8      save_ram_unlocked;

	UINT8      output_data;
	UINT8      output_latch;
	UINT8      el_value;
	UINT8      led1_value;
	UINT8      led2_value;
	UINT8      recurse;

	/* protection */
	UINT32     fatfury2_prot_data;
	UINT16     neogeo_rng;
	UINT16     *pvc_cartridge_ram;
	int        fixed_layer_bank_type;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *upd4990a;
};


/*----------- defined in drivers/neogeo.c -----------*/

void neogeo_set_display_position_interrupt_control(running_machine *machine, UINT16 data);
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

void neogeo_reset_rng(running_machine *machine);
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

VIDEO_START( neogeo );
VIDEO_RESET( neogeo );
VIDEO_UPDATE( neogeo );

READ16_HANDLER( neogeo_video_register_r );
WRITE16_HANDLER( neogeo_video_register_w );

void neogeo_set_palette_bank(running_machine *machine, UINT8 data);
void neogeo_set_screen_dark(running_machine *machine, UINT8 data);
READ16_HANDLER( neogeo_paletteram_r );
WRITE16_HANDLER( neogeo_paletteram_w );

void neogeo_set_fixed_layer_source(running_machine *machine, UINT8 data);

UINT8 neogeo_get_auto_animation_counter(running_machine *machine);
