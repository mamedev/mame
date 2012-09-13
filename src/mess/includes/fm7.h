/*
 *
 *  FM-7 header file
 *
 */

#ifndef FM7_H_
#define FM7_H_

// Interrupt flags
#define IRQ_FLAG_KEY      0x01
#define IRQ_FLAG_PRINTER  0x02
#define IRQ_FLAG_TIMER    0x04
#define IRQ_FLAG_OTHER    0x08
// the following are not read in port 0xfd03
#define IRQ_FLAG_MFD      0x10
#define IRQ_FLAG_TXRDY    0x20
#define IRQ_FLAG_RXRDY    0x40
#define IRQ_FLAG_SYNDET   0x80

// system types
#define SYS_FM7        1
#define SYS_FM77AV     2
#define SYS_FM77AV40EX 3
#define SYS_FM11       4
#define SYS_FM16       5

// keyboard scancode formats
#define KEY_MODE_FM7   0 // FM-7 ASCII type code
#define KEY_MODE_FM16B 1 // FM-16B (FM-77AV and later only)
#define KEY_MODE_SCAN  2 // Scancode Make/Break (PC-like)

typedef struct
{
	UINT8 buffer[12];
	UINT8 tx_count;
	UINT8 rx_count;
	UINT8 command_length;
	UINT8 answer_length;
	UINT8 latch;  // 0=ready to receive
	UINT8 ack;
	UINT8 position;
} fm7_encoder_t;

typedef struct
{
	UINT8 bank_addr[8][16];
	UINT8 segment;
	UINT8 window_offset;
	UINT8 enabled;
	UINT8 mode;
} fm7_mmr_t;

typedef struct
{
	UINT8 sub_busy;
	UINT8 sub_halt;
	UINT8 sub_reset;  // high if reset caused by subrom change
	UINT8 attn_irq;
	UINT8 vram_access;  // VRAM access flag
	UINT8 crt_enable;
	UINT16 vram_offset;
	UINT16 vram_offset2;
	UINT8 fm7_pal[8];
	UINT16 fm77av_pal_selected;
	UINT8 fm77av_pal_r[4096];
	UINT8 fm77av_pal_g[4096];
	UINT8 fm77av_pal_b[4096];
	UINT8 subrom;  // currently active sub CPU ROM (AV only)
	UINT8 cgrom;  // currently active CGROM (AV only)
	UINT8 modestatus;
	UINT8 multi_page;
	UINT8 fine_offset;
	UINT8 nmi_mask;
	UINT8 active_video_page;
	UINT8 display_video_page;
	UINT8 vsync_flag;
} fm7_video_t;

typedef struct
{
	UINT8 command;
	UINT8 lcolour;
	UINT8 mask;
	UINT8 compare_data;
	UINT8 compare[8];
	UINT8 bank_disable;
	UINT8 tilepaint_b;
	UINT8 tilepaint_r;
	UINT8 tilepaint_g;
	UINT16 addr_offset;
	UINT16 line_style;
	UINT16 x0;
	UINT16 x1;
	UINT16 y0;
	UINT16 y1;
	UINT8 busy;
} fm7_alu_t;


class fm7_state : public driver_device
{
public:
	fm7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_shared_ram(*this, "shared_ram"),
	m_boot_ram(*this, "boot_ram")
	{ }

	optional_shared_ptr<UINT8> m_shared_ram;
	optional_shared_ptr<UINT8> m_boot_ram;

	UINT8 m_irq_flags;
	UINT8 m_irq_mask;
	emu_timer* m_timer;
	emu_timer* m_subtimer;
	emu_timer* m_keyboard_timer;
	UINT8 m_basic_rom_en;
	UINT8 m_init_rom_en;
	unsigned int m_key_delay;
	unsigned int m_key_repeat;
	UINT16 m_current_scancode;
	UINT32 m_key_data[4];
	UINT32 m_mod_data;
	UINT8 m_key_scan_mode;
	UINT8 m_break_flag;
	UINT8 m_psg_regsel;
	UINT8 m_psg_data;
	UINT8 m_fdc_side;
	UINT8 m_fdc_drive;
	UINT8 m_fdc_irq_flag;
	UINT8 m_fdc_drq_flag;
	UINT8 m_fm77av_ym_irq;
	UINT8 m_speaker_active;
	UINT16 m_kanji_address;
	fm7_encoder_t m_encoder;
	fm7_mmr_t m_mmr;
	UINT8 m_cp_prev;
	UINT8* m_video_ram;
	emu_timer* m_fm77av_vsync_timer;
	UINT8 m_type;
	fm7_video_t m_video;
	fm7_alu_t m_alu;
	int m_sb_prev;
	DECLARE_READ8_MEMBER(fm7_subintf_r);
	DECLARE_WRITE8_MEMBER(fm7_subintf_w);
	DECLARE_READ8_MEMBER(fm7_sub_busyflag_r);
	DECLARE_WRITE8_MEMBER(fm7_sub_busyflag_w);
	DECLARE_READ8_MEMBER(fm7_cancel_ack);
	DECLARE_READ8_MEMBER(fm7_attn_irq_r);
	DECLARE_READ8_MEMBER(fm7_vram_access_r);
	DECLARE_WRITE8_MEMBER(fm7_vram_access_w);
	DECLARE_READ8_MEMBER(fm7_vram_r);
	DECLARE_WRITE8_MEMBER(fm7_vram_w);
	DECLARE_WRITE8_MEMBER(fm7_vram_banked_w);
	DECLARE_READ8_MEMBER(fm7_vram0_r);
	DECLARE_READ8_MEMBER(fm7_vram1_r);
	DECLARE_READ8_MEMBER(fm7_vram2_r);
	DECLARE_READ8_MEMBER(fm7_vram3_r);
	DECLARE_READ8_MEMBER(fm7_vram4_r);
	DECLARE_READ8_MEMBER(fm7_vram5_r);
	DECLARE_READ8_MEMBER(fm7_vram6_r);
	DECLARE_READ8_MEMBER(fm7_vram7_r);
	DECLARE_READ8_MEMBER(fm7_vram8_r);
	DECLARE_READ8_MEMBER(fm7_vram9_r);
	DECLARE_READ8_MEMBER(fm7_vramA_r);
	DECLARE_READ8_MEMBER(fm7_vramB_r);
	DECLARE_WRITE8_MEMBER(fm7_vram0_w);
	DECLARE_WRITE8_MEMBER(fm7_vram1_w);
	DECLARE_WRITE8_MEMBER(fm7_vram2_w);
	DECLARE_WRITE8_MEMBER(fm7_vram3_w);
	DECLARE_WRITE8_MEMBER(fm7_vram4_w);
	DECLARE_WRITE8_MEMBER(fm7_vram5_w);
	DECLARE_WRITE8_MEMBER(fm7_vram6_w);
	DECLARE_WRITE8_MEMBER(fm7_vram7_w);
	DECLARE_WRITE8_MEMBER(fm7_vram8_w);
	DECLARE_WRITE8_MEMBER(fm7_vram9_w);
	DECLARE_WRITE8_MEMBER(fm7_vramA_w);
	DECLARE_WRITE8_MEMBER(fm7_vramB_w);
	DECLARE_READ8_MEMBER(fm7_crt_r);
	DECLARE_WRITE8_MEMBER(fm7_crt_w);
	DECLARE_WRITE8_MEMBER(fm7_vram_offset_w);
	DECLARE_WRITE8_MEMBER(fm7_multipage_w);
	DECLARE_READ8_MEMBER(fm7_palette_r);
	DECLARE_WRITE8_MEMBER(fm7_palette_w);
	DECLARE_WRITE8_MEMBER(fm77av_analog_palette_w);
	DECLARE_READ8_MEMBER(fm77av_video_flags_r);
	DECLARE_WRITE8_MEMBER(fm77av_video_flags_w);
	DECLARE_READ8_MEMBER(fm77av_sub_modestatus_r);
	DECLARE_WRITE8_MEMBER(fm77av_sub_modestatus_w);
	DECLARE_WRITE8_MEMBER(fm77av_sub_bank_w);
	DECLARE_READ8_MEMBER(fm77av_alu_r);
	DECLARE_WRITE8_MEMBER(fm77av_alu_w);
	DECLARE_READ8_MEMBER(fm7_sub_ram_ports_banked_r);
	DECLARE_WRITE8_MEMBER(fm7_sub_ram_ports_banked_w);
	DECLARE_READ8_MEMBER(fm7_console_ram_banked_r);
	DECLARE_WRITE8_MEMBER(fm7_console_ram_banked_w);
	DECLARE_WRITE8_MEMBER(fm7_irq_mask_w);
	DECLARE_READ8_MEMBER(fm7_irq_cause_r);
	DECLARE_WRITE8_MEMBER(fm7_beeper_w);
	DECLARE_READ8_MEMBER(fm7_sub_beeper_r);
	DECLARE_READ8_MEMBER(vector_r);
	DECLARE_WRITE8_MEMBER(vector_w);
	DECLARE_READ8_MEMBER(fm7_fd04_r);
	DECLARE_READ8_MEMBER(fm7_rom_en_r);
	DECLARE_WRITE8_MEMBER(fm7_rom_en_w);
	DECLARE_WRITE8_MEMBER(fm7_init_en_w);
	DECLARE_READ8_MEMBER(fm7_fdc_r);
	DECLARE_WRITE8_MEMBER(fm7_fdc_w);
	DECLARE_READ8_MEMBER(fm7_keyboard_r);
	DECLARE_READ8_MEMBER(fm7_sub_keyboard_r);
	DECLARE_READ8_MEMBER(fm77av_key_encoder_r);
	DECLARE_WRITE8_MEMBER(fm77av_key_encoder_w);
	DECLARE_READ8_MEMBER(fm7_cassette_printer_r);
	DECLARE_WRITE8_MEMBER(fm7_cassette_printer_w);
	DECLARE_READ8_MEMBER(fm77av_boot_mode_r);
	DECLARE_READ8_MEMBER(fm7_psg_select_r);
	DECLARE_WRITE8_MEMBER(fm7_psg_select_w);
	DECLARE_WRITE8_MEMBER(fm77av_ym_select_w);
	DECLARE_READ8_MEMBER(fm7_psg_data_r);
	DECLARE_WRITE8_MEMBER(fm7_psg_data_w);
	DECLARE_WRITE8_MEMBER(fm77av_bootram_w);
	DECLARE_READ8_MEMBER(fm7_main_shared_r);
	DECLARE_WRITE8_MEMBER(fm7_main_shared_w);
	DECLARE_READ8_MEMBER(fm7_fmirq_r);
	DECLARE_READ8_MEMBER(fm7_unknown_r);
	DECLARE_READ8_MEMBER(fm7_mmr_r);
	DECLARE_WRITE8_MEMBER(fm7_mmr_w);
	DECLARE_READ8_MEMBER(fm7_kanji_r);
	DECLARE_WRITE8_MEMBER(fm7_kanji_w);
	void fm77av_encoder_setup_command();
	void fm77av_encoder_handle_command();
	DECLARE_DRIVER_INIT(fm7);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(fm7);
	DECLARE_MACHINE_START(fm77av);
	DECLARE_MACHINE_START(fm11);
	DECLARE_MACHINE_START(fm16);
};


/*----------- defined in drivers/fm7.c -----------*/



/*----------- defined in video/fm7.c -----------*/

TIMER_CALLBACK( fm77av_vsync );








SCREEN_UPDATE_IND16( fm7 );


#endif /*FM7_H_*/
