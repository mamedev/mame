// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "sound/2203intf.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"

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

struct fm7_encoder_t
{
	UINT8 buffer[12];
	UINT8 tx_count;
	UINT8 rx_count;
	UINT8 command_length;
	UINT8 answer_length;
	UINT8 latch;  // 0=ready to receive
	UINT8 ack;
	UINT8 position;
};

struct fm7_mmr_t
{
	UINT8 bank_addr[8][16];
	UINT8 segment;
	UINT8 window_offset;
	UINT8 enabled;
	UINT8 mode;
};

struct fm7_video_t
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
	UINT8 subrom;  // currently active sub CPU ROM (AV only)
	UINT8 cgrom;  // currently active CGROM (AV only)
	UINT8 modestatus;
	UINT8 multi_page;
	UINT8 fine_offset;
	UINT8 nmi_mask;
	UINT8 active_video_page;
	UINT8 display_video_page;
	UINT8 vsync_flag;
};

struct fm7_alu_t
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
};


class fm7_state : public driver_device
{
public:
	enum
	{
		TIMER_FM7_BEEPER_OFF,
		TIMER_FM77AV_ENCODER_ACK,
		TIMER_FM7_IRQ,
		TIMER_FM7_SUBTIMER_IRQ,
		TIMER_FM7_KEYBOARD_POLL,
		TIMER_FM77AV_ALU_TASK_END,
		TIMER_FM77AV_VSYNC
	};

	fm7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_boot_ram(*this, "boot_ram"),
		m_maincpu(*this, "maincpu"),
		m_sub(*this, "sub"),
		m_x86(*this, "x86"),
		m_cassette(*this, "cassette"),
		m_beeper(*this, "beeper"),
		m_ym(*this, "ym"),
		m_psg(*this, "psg"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_floppy(nullptr),
		m_ram_ptr(*this, "maincpu"),
		m_rom_ptr(*this, "init"),
		m_basic_ptr(*this, "fbasic"),
		m_kanji(*this, "kanji1"),
		m_key1(*this, "key1"),
		m_key2(*this, "key2"),
		m_key3(*this, "key3"),
		m_keymod(*this, "key_modifiers"),
		m_joy1(*this, "joy1"),
		m_joy2(*this, "joy2"),
		m_dsw(*this, "DSW"),
		m_palette(*this, "palette"),
		m_av_palette(*this, "av_palette"),
		m_avbank1(*this, "av_bank1"),
		m_avbank2(*this, "av_bank2"),
		m_avbank3(*this, "av_bank3"),
		m_avbank4(*this, "av_bank4"),
		m_avbank5(*this, "av_bank5"),
		m_avbank6(*this, "av_bank6"),
		m_avbank7(*this, "av_bank7"),
		m_avbank8(*this, "av_bank8"),
		m_avbank9(*this, "av_bank9"),
		m_avbank10(*this, "av_bank10"),
		m_avbank11(*this, "av_bank11"),
		m_avbank12(*this, "av_bank12"),
		m_avbank13(*this, "av_bank13"),
		m_avbank14(*this, "av_bank14"),
		m_avbank15(*this, "av_bank15"),
		m_avbank16(*this, "av_bank16")
	{
	}
	DECLARE_DRIVER_INIT(fm7);

	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_MACHINE_START(fm7);
	DECLARE_MACHINE_START(fm77av);
	DECLARE_MACHINE_START(fm11);
	DECLARE_MACHINE_START(fm16);

	DECLARE_WRITE_LINE_MEMBER(fm7_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fm7_fdc_drq_w);
	DECLARE_READ8_MEMBER(fm77av_joy_1_r);
	DECLARE_READ8_MEMBER(fm77av_joy_2_r);
	DECLARE_WRITE_LINE_MEMBER(fm77av_fmirq);

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

	IRQ_CALLBACK_MEMBER(fm7_irq_ack);
	IRQ_CALLBACK_MEMBER(fm7_sub_irq_ack);

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_fault);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);

	UINT32 screen_update_fm7(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	optional_shared_ptr<UINT8> m_shared_ram;
	optional_shared_ptr<UINT8> m_boot_ram;

	UINT8           m_irq_flags;
	UINT8           m_irq_mask;
	emu_timer*      m_timer;
	emu_timer*      m_subtimer;
	emu_timer*      m_keyboard_timer;
	UINT8           m_basic_rom_en;
	UINT8           m_init_rom_en;

	unsigned int    m_key_delay;
	unsigned int    m_key_repeat;
	UINT16          m_current_scancode;
	UINT32          m_key_data[4];
	UINT32          m_mod_data;
	UINT8           m_key_scan_mode;
	UINT8           m_break_flag;

	UINT8           m_psg_regsel;
	UINT8           m_psg_data;

	UINT8           m_fdc_side;
	UINT8           m_fdc_drive;
	UINT8           m_fdc_irq_flag;
	UINT8           m_fdc_drq_flag;

	UINT8           m_fm77av_ym_irq;
	UINT8           m_speaker_active;

	UINT16          m_kanji_address;
	fm7_encoder_t   m_encoder;
	fm7_mmr_t       m_mmr;
	UINT8           m_cp_prev;

	std::unique_ptr<UINT8[]>    m_video_ram;
	emu_timer*                  m_fm77av_vsync_timer;
	UINT8 m_type;
	fm7_video_t     m_video;
	fm7_alu_t       m_alu;
	int             m_sb_prev;

	void fm77av_encoder_setup_command();
	void fm77av_encoder_handle_command();
	TIMER_CALLBACK_MEMBER(fm7_beeper_off);
	TIMER_CALLBACK_MEMBER(fm77av_encoder_ack);
	TIMER_CALLBACK_MEMBER(fm7_timer_irq);
	TIMER_CALLBACK_MEMBER(fm7_subtimer_irq);
	TIMER_CALLBACK_MEMBER(fm7_keyboard_poll);
	TIMER_CALLBACK_MEMBER(fm77av_alu_task_end);
	TIMER_CALLBACK_MEMBER(fm77av_vsync);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_sub;
	optional_device<cpu_device> m_x86;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	optional_device<ym2203_device> m_ym;
	optional_device<ay8910_device> m_psg;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	optional_region_ptr<UINT8> m_ram_ptr;
	optional_region_ptr<UINT8> m_rom_ptr;
	optional_region_ptr<UINT8> m_basic_ptr;

	void fm7_alu_mask_write(UINT32 offset, int bank, UINT8 dat);
	void fm7_alu_function_compare(UINT32 offset);
	void fm7_alu_function_pset(UINT32 offset);
	void fm7_alu_function_or(UINT32 offset);
	void fm7_alu_function_and(UINT32 offset);
	void fm7_alu_function_xor(UINT32 offset);
	void fm7_alu_function_not(UINT32 offset);
	void fm7_alu_function_invalid(UINT32 offset);
	void fm7_alu_function_tilepaint(UINT32 offset);
	void fm7_alu_function(UINT32 offset);
	UINT32 fm7_line_set_pixel(int x, int y);
	void fm77av_line_draw();
	void main_irq_set_flag(UINT8 flag);
	void main_irq_clear_flag(UINT8 flag);
	void fm7_update_psg();
	void fm7_update_bank(address_space & space, int bank, UINT8 physical);
	void fm7_mmr_refresh(address_space& space);
	void key_press(UINT16 scancode);
	void fm7_keyboard_poll_scan();

	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_ack;
	int m_centronics_perror;

	optional_memory_region m_kanji;
	required_ioport m_key1;
	required_ioport m_key2;
	required_ioport m_key3;
	required_ioport m_keymod;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_dsw;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_av_palette;

	optional_device<address_map_bank_device> m_avbank1;
	optional_device<address_map_bank_device> m_avbank2;
	optional_device<address_map_bank_device> m_avbank3;
	optional_device<address_map_bank_device> m_avbank4;
	optional_device<address_map_bank_device> m_avbank5;
	optional_device<address_map_bank_device> m_avbank6;
	optional_device<address_map_bank_device> m_avbank7;
	optional_device<address_map_bank_device> m_avbank8;
	optional_device<address_map_bank_device> m_avbank9;
	optional_device<address_map_bank_device> m_avbank10;
	optional_device<address_map_bank_device> m_avbank11;
	optional_device<address_map_bank_device> m_avbank12;
	optional_device<address_map_bank_device> m_avbank13;
	optional_device<address_map_bank_device> m_avbank14;
	optional_device<address_map_bank_device> m_avbank15;
	optional_device<address_map_bank_device> m_avbank16;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /*FM7_H_*/
