/*************************************************************************

    Neo-Geo hardware

*************************************************************************/

#include "machine/upd1990a.h"

#define NEOGEO_MASTER_CLOCK                     (24000000)
#define NEOGEO_MAIN_CPU_CLOCK                   (NEOGEO_MASTER_CLOCK / 2)
#define NEOGEO_AUDIO_CPU_CLOCK                  (NEOGEO_MASTER_CLOCK / 6)
#define NEOGEO_YM2610_CLOCK                     (NEOGEO_MASTER_CLOCK / 3)
#define NEOGEO_PIXEL_CLOCK                      (NEOGEO_MASTER_CLOCK / 4)
#define NEOGEO_HTOTAL                           (0x180)
#define NEOGEO_HBEND                            (0x01e) /* this should really be 29.5 */
#define NEOGEO_HBSTART                          (0x15e) /* this should really be 349.5 */
#define NEOGEO_VTOTAL                           (0x108)
#define NEOGEO_VBEND                            (0x010)
#define NEOGEO_VBSTART                          (0x0f0)
#define NEOGEO_VSSTART                          (0x100)


class neogeo_state : public driver_device
{
public:
	neogeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_region_maincpu(*this, "maincpu"),
		m_region_sprites(*this, "sprites"),
		m_region_fixed(*this, "fixed"),
		m_region_fixedbios(*this, "fixedbios"),
		m_bank_vectors(*this, "vectors"),
		m_bank_cartridge(*this, "cartridge"),
		m_bank_audio_main(*this, "audio_main"),
		m_upd4990a(*this, "upd4990a"),
		m_save_ram(*this, "saveram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	DECLARE_WRITE8_MEMBER(io_control_w);
	DECLARE_READ16_MEMBER(memcard_r);
	DECLARE_WRITE16_MEMBER(memcard_w);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_WRITE16_MEMBER(main_cpu_bank_select_w);
	DECLARE_READ8_MEMBER(audio_cpu_bank_select_r);
	DECLARE_WRITE8_MEMBER(audio_cpu_enable_nmi_w);
	DECLARE_WRITE8_MEMBER(system_control_w);
	DECLARE_READ16_MEMBER(neogeo_unmapped_r);
	DECLARE_READ16_MEMBER(neogeo_paletteram_r);
	DECLARE_WRITE16_MEMBER(neogeo_paletteram_w);
	DECLARE_READ16_MEMBER(neogeo_video_register_r);
	DECLARE_WRITE16_MEMBER(neogeo_video_register_w);

	DECLARE_CUSTOM_INPUT_MEMBER(get_memcard_status);
	DECLARE_CUSTOM_INPUT_MEMBER(get_audio_result);

	DECLARE_DRIVER_INIT(neogeo);
	DECLARE_DRIVER_INIT(fatfury2);
	DECLARE_DRIVER_INIT(zupapa);
	DECLARE_DRIVER_INIT(kof98);
	DECLARE_DRIVER_INIT(mslugx);
	DECLARE_DRIVER_INIT(kof99);
	DECLARE_DRIVER_INIT(kof99k);
	DECLARE_DRIVER_INIT(garou);
	DECLARE_DRIVER_INIT(garouh);
	DECLARE_DRIVER_INIT(garoubl);
	DECLARE_DRIVER_INIT(mslug3);
	DECLARE_DRIVER_INIT(mslug3h);
	DECLARE_DRIVER_INIT(mslug3b6);
	DECLARE_DRIVER_INIT(kof2000);
	DECLARE_DRIVER_INIT(kof2000n);
	DECLARE_DRIVER_INIT(kof2001);
	DECLARE_DRIVER_INIT(cthd2003);
	DECLARE_DRIVER_INIT(ct2k3sp);
	DECLARE_DRIVER_INIT(ct2k3sa);
	DECLARE_DRIVER_INIT(mslug4);
	DECLARE_DRIVER_INIT(ms4plus);
	DECLARE_DRIVER_INIT(ganryu);
	DECLARE_DRIVER_INIT(s1945p);
	DECLARE_DRIVER_INIT(preisle2);
	DECLARE_DRIVER_INIT(bangbead);
	DECLARE_DRIVER_INIT(nitd);
	DECLARE_DRIVER_INIT(sengoku3);
	DECLARE_DRIVER_INIT(rotd);
	DECLARE_DRIVER_INIT(kof2002);
	DECLARE_DRIVER_INIT(kof2002b);
	DECLARE_DRIVER_INIT(kf2k2pls);
	DECLARE_DRIVER_INIT(kf2k2mp);
	DECLARE_DRIVER_INIT(kf2k2mp2);
	DECLARE_DRIVER_INIT(kof10th);
	DECLARE_DRIVER_INIT(kf10thep);
	DECLARE_DRIVER_INIT(kf2k5uni);
	DECLARE_DRIVER_INIT(kof2k4se);
	DECLARE_DRIVER_INIT(matrim);
	DECLARE_DRIVER_INIT(matrimbl);
	DECLARE_DRIVER_INIT(pnyaa);
	DECLARE_DRIVER_INIT(mslug5);
	DECLARE_DRIVER_INIT(ms5pcb);
	DECLARE_DRIVER_INIT(ms5plus);
	DECLARE_DRIVER_INIT(svcpcb);
	DECLARE_DRIVER_INIT(svc);
	DECLARE_DRIVER_INIT(svcboot);
	DECLARE_DRIVER_INIT(svcplus);
	DECLARE_DRIVER_INIT(svcplusa);
	DECLARE_DRIVER_INIT(svcsplus);
	DECLARE_DRIVER_INIT(samsho5);
	DECLARE_DRIVER_INIT(samsho5b);
	DECLARE_DRIVER_INIT(kf2k3pcb);
	DECLARE_DRIVER_INIT(kof2003);
	DECLARE_DRIVER_INIT(kof2003h);
	DECLARE_DRIVER_INIT(kf2k3bl);
	DECLARE_DRIVER_INIT(kf2k3pl);
	DECLARE_DRIVER_INIT(kf2k3upl);
	DECLARE_DRIVER_INIT(samsh5sp);
	DECLARE_DRIVER_INIT(jockeygp);
	DECLARE_DRIVER_INIT(vliner);
	DECLARE_DRIVER_INIT(kog);
	DECLARE_DRIVER_INIT(kof97oro);
	DECLARE_DRIVER_INIT(lans2004);
	DECLARE_DRIVER_INIT(sbp);
	DECLARE_DRIVER_INIT(mvs);
	void mvs_install_protection(device_image_interface& image);

	TIMER_CALLBACK_MEMBER(display_position_interrupt_callback);
	TIMER_CALLBACK_MEMBER(display_position_vblank_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);
	TIMER_CALLBACK_MEMBER(auto_animation_timer_callback);
	TIMER_CALLBACK_MEMBER(sprite_line_timer_callback);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(neo_cartridge);

	// MVS-specific
	DECLARE_WRITE16_MEMBER(save_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(multiplexed_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kizuna4p_controller_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kizuna4p_start_r);
	DECLARE_INPUT_CHANGED_MEMBER(select_bios);

	UINT32 screen_update_neogeo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// this has to be public for the legacy MEMCARD_HANDLER
	UINT8      *m_memcard_data;

protected:
	void neogeo_postload();
	void update_interrupts();
	void create_interrupt_timers();
	void start_interrupt_timers();
	void neogeo_acknowledge_interrupt(UINT16 data);
	void neogeo_set_main_cpu_bank_address( UINT32 bank_address );
	void _set_main_cpu_bank_address();
	void neogeo_main_cpu_banking_init();
	void neogeo_audio_cpu_banking_init();
	void adjust_display_position_interrupt_timer();
	void neogeo_set_display_position_interrupt_control(UINT16 data);
	void neogeo_set_display_counter_msb(UINT16 data);
	void neogeo_set_display_counter_lsb(UINT16 data);
	void set_video_control( UINT16 data );
	void optimize_sprite_data();
	void draw_fixed_layer( bitmap_rgb32 &bitmap, int scanline );
	void set_videoram_offset( UINT16 data );
	UINT16 get_videoram_data(  );
	void set_videoram_data( UINT16 data);
	void set_videoram_modulo( UINT16 data);
	UINT16 get_videoram_modulo(  );
	void compute_rgb_weights(  );
	void regenerate_pens();
	pen_t get_pen( UINT16 data );
	void neogeo_set_palette_bank( UINT8 data );
	void neogeo_set_screen_dark( UINT8 data );
	void set_auto_animation_speed( UINT8 data);
	void set_auto_animation_disabled( UINT8 data);
	UINT8 neogeo_get_auto_animation_counter(  );
	void create_auto_animation_timer(  );
	void start_auto_animation_timer(  );
	void neogeo_set_fixed_layer_source( UINT8 data );
	inline int rows_to_height(int rows);
	inline int sprite_on_scanline(int scanline, int y, int rows);
	void draw_sprites( bitmap_rgb32 &bitmap, int scanline );
	void parse_sprites( int scanline );
	void create_sprite_line_timer(  );
	void start_sprite_line_timer(  );
	UINT16 get_video_control(  );
	void audio_cpu_check_nmi();
	void select_controller( UINT8 data );
	void set_save_ram_unlock( UINT8 data );
	void set_outputs(  );
	void set_output_latch( UINT8 data );
	void set_output_data( UINT8 data );
	void install_banked_bios();

	// protections implementation
	DECLARE_READ16_MEMBER( sbp_lowerrom_r );
	DECLARE_WRITE16_MEMBER( sbp_lowerrom_w );
	DECLARE_READ16_MEMBER( fatfury2_protection_16_r );
	DECLARE_WRITE16_MEMBER( fatfury2_protection_16_w );
	void fatfury2_install_protection();
	DECLARE_WRITE16_MEMBER ( kof98_prot_w );
	void install_kof98_protection();
	DECLARE_WRITE16_MEMBER( mslugx_protection_16_w );
	DECLARE_READ16_MEMBER( mslugx_protection_16_r );
	void mslugx_install_protection();
	DECLARE_WRITE16_MEMBER( kof99_bankswitch_w );
	DECLARE_WRITE16_MEMBER( garou_bankswitch_w );
	DECLARE_WRITE16_MEMBER( garouh_bankswitch_w );
	DECLARE_WRITE16_MEMBER( mslug3_bankswitch_w );
	DECLARE_WRITE16_MEMBER( kof2000_bankswitch_w );
	DECLARE_READ16_MEMBER( prot_9a37_r );
	DECLARE_READ16_MEMBER( sma_random_r );
	void reset_sma_rng();
	void sma_install_random_read_handler( int addr1, int addr2 );
	void kof99_install_protection();
	void garou_install_protection();
	void garouh_install_protection();
	void mslug3_install_protection();
	void kof2000_install_protection();
	void pvc_write_unpack_color();
	void pvc_write_pack_color();
	void pvc_write_bankswitch( address_space &space );
	DECLARE_READ16_MEMBER( pvc_prot_r );
	DECLARE_WRITE16_MEMBER( pvc_prot_w );
	void install_pvc_protection();
	void neogeo_bootleg_cx_decrypt();
	void neogeo_bootleg_sx_decrypt(int value );
	void kog_px_decrypt();
	void kof97oro_px_decode();
	void kof10thBankswitch(address_space &space, UINT16 nBank);
	DECLARE_READ16_MEMBER( kof10th_RAMB_r );
	DECLARE_WRITE16_MEMBER( kof10th_custom_w );
	DECLARE_WRITE16_MEMBER( kof10th_bankswitch_w );
	void install_kof10th_protection ();
	void decrypt_kof10th();
	void kf10thep_px_decrypt();
	void kf2k5uni_px_decrypt();
	void kf2k5uni_sx_decrypt();
	void kf2k5uni_mx_decrypt();
	void decrypt_kf2k5uni();
	void kof2002b_gfx_decrypt(UINT8 *src, int size);
	void kf2k2mp_decrypt();
	void kf2k2mp2_px_decrypt();
	void cthd2003_neogeo_gfx_address_fix_do(int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift);
	void cthd2003_neogeo_gfx_address_fix(int start, int end);
	void cthd2003_c(int pow);
	void decrypt_cthd2003();
	DECLARE_WRITE16_MEMBER ( cthd2003_bankswitch_w );
	void patch_cthd2003();
	void ct2k3sp_sx_decrypt();
	void decrypt_ct2k3sp();
	void decrypt_ct2k3sa();
	void patch_ct2k3sa();
	void decrypt_kof2k4se_68k();
	void lans2004_vx_decrypt();
	void lans2004_decrypt_68k();
	DECLARE_READ16_MEMBER( mslug5_prot_r );
	DECLARE_WRITE16_MEMBER ( ms5plus_bankswitch_w );
	void install_ms5plus_protection();
	void svcboot_px_decrypt();
	void svcboot_cx_decrypt();
	void svcplus_px_decrypt();
	void svcplus_px_hack();
	void svcplusa_px_decrypt();
	void svcsplus_px_decrypt();
	void svcsplus_px_hack();
	DECLARE_READ16_MEMBER( kof2003_r);
	DECLARE_WRITE16_MEMBER( kof2003_w );
	DECLARE_WRITE16_MEMBER( kof2003p_w );
	void kf2k3bl_px_decrypt();
	void kf2k3bl_install_protection();
	void kf2k3pl_px_decrypt();
	void kf2k3pl_install_protection();
	void kf2k3upl_px_decrypt();
	void samsho5b_px_decrypt();
	void samsho5b_vx_decrypt();
	void matrimbl_decrypt();
	void decrypt(UINT8 *r0, UINT8 *r1,UINT8 c0,  UINT8 c1,const UINT8 *table0hi,const UINT8 *table0lo,const UINT8 *table1,int base,int invert);
	void neogeo_gfx_decrypt(int extra_xor);
	void neogeo_sfix_decrypt();
	void kof99_neogeo_gfx_decrypt(int extra_xor);
	void kof2000_neogeo_gfx_decrypt(int extra_xor);
	void cmc42_neogeo_gfx_decrypt(int extra_xor);
	void cmc50_neogeo_gfx_decrypt(int extra_xor);
	void svcpcb_gfx_decrypt();
	void svcpcb_s1data_decrypt();
	void kf2k3pcb_gfx_decrypt();
	void kf2k3pcb_decrypt_s1data();
	UINT16 generate_cs16(UINT8 *rom, int size);
	int m1_address_scramble(int address, UINT16 key);
	void neogeo_cmc50_m1_decrypt();
	void kof98_decrypt_68k();
	void kof99_decrypt_68k();
	void garou_decrypt_68k();
	void garouh_decrypt_68k();
	void mslug3_decrypt_68k();
	void kof2000_decrypt_68k();
	void kof2002_decrypt_68k();
	void matrim_decrypt_68k();
	void samsho5_decrypt_68k();
	void samsh5sp_decrypt_68k();
	void mslug5_decrypt_68k();
	void svc_px_decrypt();
	void kf2k3pcb_decrypt_68k();
	void kof2003_decrypt_68k();
	void kof2003h_decrypt_68k();
	void neo_pcm2_snk_1999(int value);
	void neo_pcm2_swap(int value);
	void kf2k3pcb_sp1_decrypt();

	// device overrides
	virtual void machine_start();
	virtual void machine_reset();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	// memory
	required_memory_region m_region_maincpu;
	required_memory_region m_region_sprites;
	required_memory_region m_region_fixed;
	optional_memory_region m_region_fixedbios;
	required_memory_bank   m_bank_vectors;
	optional_memory_bank   m_bank_cartridge;  // optional because of neocd
	optional_memory_bank   m_bank_audio_main; // optional because of neocd
	memory_bank           *m_bank_audio_cart[4];

	// MVS-specific devices
	optional_device<upd4990a_device> m_upd4990a;
	optional_shared_ptr<UINT16> m_save_ram;

	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	// configuration
	enum {NEOGEO_MVS, NEOGEO_AES, NEOGEO_CD} m_type;

	// internal state
	UINT32     m_main_cpu_bank_address;
	UINT8      m_controller_select;
	bool       m_recurse;
	bool       m_audio_cpu_nmi_enabled;
	bool       m_audio_cpu_nmi_pending;

	// MVS-specific state
	UINT8      m_save_ram_unlocked;
	UINT8      m_output_data;
	UINT8      m_output_latch;
	UINT8      m_el_value;
	UINT8      m_led1_value;
	UINT8      m_led2_value;

	// video hardware, including maincpu interrupts
	// TODO: make into a device
	virtual void video_start();
	virtual void video_reset();

	emu_timer  *m_display_position_interrupt_timer;
	emu_timer  *m_display_position_vblank_timer;
	emu_timer  *m_vblank_interrupt_timer;
	emu_timer  *m_auto_animation_timer;
	emu_timer  *m_sprite_line_timer;
	UINT32     m_display_counter;
	UINT8      m_vblank_interrupt_pending;
	UINT8      m_display_position_interrupt_pending;
	UINT8      m_irq3_pending;
	UINT8      m_display_position_interrupt_control;
	UINT8      m_vblank_level;
	UINT8      m_raster_level;

	UINT16     *m_videoram;
	UINT16     m_vram_offset;
	UINT16     m_vram_read_buffer;
	UINT16     m_vram_modulo;

	const UINT8 *m_region_zoomy;

	dynamic_array<UINT8> m_sprite_gfx;
	UINT32     m_sprite_gfx_address_mask;

	UINT8      m_auto_animation_speed;
	UINT8      m_auto_animation_disabled;
	UINT8      m_auto_animation_counter;
	UINT8      m_auto_animation_frame_counter;

	UINT8      m_fixed_layer_source;
	UINT8      m_fixed_layer_bank_type;

	// color/palette related
	// TODO: disentangle from the rest of the video emulation
	double     m_rgb_weights_normal[5];
	double     m_rgb_weights_normal_bit15[5];
	double     m_rgb_weights_dark[5];
	double     m_rgb_weights_dark_bit15[5];
	UINT16     *m_palettes[2]; /* 0x100*16 2 byte palette entries */
	pen_t      *m_pens;
	UINT8      m_palette_bank;
	UINT8      m_screen_dark;

	// cartridge-specific hardware
	// TODO: move into separate devices
	UINT32     m_fatfury2_prot_data;
	UINT16     m_sma_rng;
	UINT16     m_mslugx_counter;
	UINT16     m_mslugx_command;

	const UINT8 *type0_t03;
	const UINT8 *type0_t12;
	const UINT8 *type1_t03;
	const UINT8 *type1_t12;
	const UINT8 *address_8_15_xor1;
	const UINT8 *address_8_15_xor2;
	const UINT8 *address_16_23_xor1;
	const UINT8 *address_16_23_xor2;
	const UINT8 *address_0_7_xor;

	UINT16 m_cartridge_ram[0x1000];
};


/*----------- defined in drivers/neogeo.c -----------*/

MACHINE_CONFIG_EXTERN( neogeo_base );
MEMCARD_HANDLER( neogeo );
