// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Barry Rodewald
/*****************************************************************************
 *
 * includes/x1.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_X1_H
#define MAME_INCLUDES_X1_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "sound/ay8910.h"
#include "sound/ym2151.h"
#include "video/mc6845.h"

#include "formats/x1_tap.h"

#include "emupal.h"
#include "screen.h"


// ======================> x1_keyboard_device

class x1_keyboard_device :  public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	x1_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_start() override;
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;
};

class x1_state : public driver_device
{
public:
	x1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "x1_cpu"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dma(*this, "dma"),
		m_iobank(*this, "iobank"),
		m_ym(*this, "ym"),
		m_sound_sw(*this, "SOUND_SW"),
		m_tvram(*this, "tvram"),
		m_avram(*this, "avram"),
		m_kvram(*this, "kvram"),
		m_bitmapbank(*this, "bitmapbank"),
		m_ipl_rom(*this, "ipl"),
		m_cg_rom(*this, "cgrom"),
		m_kanji_rom(*this, "kanji")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<mb8877_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;

	DECLARE_READ8_MEMBER(x1_mem_r);
	DECLARE_WRITE8_MEMBER(x1_mem_w);
	DECLARE_READ8_MEMBER(x1_sub_io_r);
	DECLARE_WRITE8_MEMBER(x1_sub_io_w);
	DECLARE_READ8_MEMBER(x1_rom_r);
	DECLARE_WRITE8_MEMBER(x1_rom_w);
	DECLARE_WRITE8_MEMBER(x1_rom_bank_0_w);
	DECLARE_WRITE8_MEMBER(x1_rom_bank_1_w);
	DECLARE_READ8_MEMBER(x1_fdc_r);
	DECLARE_WRITE8_MEMBER(x1_fdc_w);
	DECLARE_READ8_MEMBER(x1_pcg_r);
	DECLARE_WRITE8_MEMBER(x1_pcg_w);
	DECLARE_WRITE8_MEMBER(x1_pal_r_w);
	DECLARE_WRITE8_MEMBER(x1_pal_g_w);
	DECLARE_WRITE8_MEMBER(x1_pal_b_w);
	DECLARE_READ8_MEMBER(x1_ex_gfxram_r);
	DECLARE_WRITE8_MEMBER(x1_ex_gfxram_w);
	DECLARE_WRITE8_MEMBER(x1_scrn_w);
	DECLARE_WRITE8_MEMBER(x1_pri_w);
	DECLARE_WRITE8_MEMBER(x1_6845_w);
	DECLARE_READ8_MEMBER(x1_kanji_r);
	DECLARE_WRITE8_MEMBER(x1_kanji_w);
	DECLARE_READ8_MEMBER(x1_emm_r);
	DECLARE_WRITE8_MEMBER(x1_emm_w);
	DECLARE_READ8_MEMBER(x1turbo_pal_r);
	DECLARE_READ8_MEMBER(x1turbo_txpal_r);
	DECLARE_READ8_MEMBER(x1turbo_txdisp_r);
	DECLARE_READ8_MEMBER(x1turbo_gfxpal_r);
	DECLARE_WRITE8_MEMBER(x1turbo_pal_w);
	DECLARE_WRITE8_MEMBER(x1turbo_txpal_w);
	DECLARE_WRITE8_MEMBER(x1turbo_txdisp_w);
	DECLARE_WRITE8_MEMBER(x1turbo_gfxpal_w);
	DECLARE_WRITE8_MEMBER(x1turbo_blackclip_w);
	DECLARE_READ8_MEMBER(x1turbo_mem_r);
	DECLARE_WRITE8_MEMBER(x1turbo_mem_w);
	DECLARE_WRITE8_MEMBER(x1turboz_4096_palette_w);
	DECLARE_READ8_MEMBER(x1turboz_blackclip_r);
	DECLARE_READ8_MEMBER(x1turbo_bank_r);
	DECLARE_WRITE8_MEMBER(x1turbo_bank_w);
	DECLARE_READ8_MEMBER(x1_porta_r);
	DECLARE_READ8_MEMBER(x1_portb_r);
	DECLARE_READ8_MEMBER(x1_portc_r);
	DECLARE_WRITE8_MEMBER(x1_porta_w);
	DECLARE_WRITE8_MEMBER(x1_portb_w);
	DECLARE_WRITE8_MEMBER(x1_portc_w);
	void init_x1_kanji();
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(x1);
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(x1turbo);
	uint32_t screen_update_x1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_INPUT_CHANGED_MEMBER(ipl_reset);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset);
	TIMER_CALLBACK_MEMBER(x1_rtc_increment);
	TIMER_DEVICE_CALLBACK_MEMBER(x1_cmt_wind_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(x1_keyboard_callback);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(hdl_w);

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<z80dma_device> m_dma;
	void x1turbo(machine_config &config);
	void x1(machine_config &config);

	DECLARE_READ8_MEMBER(ym_r);
	DECLARE_READ8_MEMBER(color_board_r);
	DECLARE_WRITE8_MEMBER(color_board_w);
	DECLARE_READ8_MEMBER(color_board_2_r);
	DECLARE_WRITE8_MEMBER(color_board_2_w);
	DECLARE_READ8_MEMBER(stereo_board_r);
	DECLARE_WRITE8_MEMBER(stereo_board_w);
	DECLARE_READ8_MEMBER(rs232_r);
	DECLARE_WRITE8_MEMBER(rs232_w);
	DECLARE_READ8_MEMBER(sasi_r);
	DECLARE_WRITE8_MEMBER(sasi_w);
	DECLARE_READ8_MEMBER(fdd8_r);
	DECLARE_WRITE8_MEMBER(fdd8_w);
	DECLARE_READ8_MEMBER(ext_sio_ctc_r);
	DECLARE_WRITE8_MEMBER(ext_sio_ctc_w);
	DECLARE_WRITE8_MEMBER(z_img_cap_w);
	DECLARE_WRITE8_MEMBER(z_mosaic_w);
	DECLARE_WRITE8_MEMBER(z_chroma_key_w);
	DECLARE_WRITE8_MEMBER(z_extra_scroll_w);

	uint8_t m_key_irq_flag;       /**< Keyboard IRQ pending. */
	uint8_t m_key_irq_vector;     /**< Keyboard IRQ vector. */

	void x1_io(address_map &map);
	void x1_io_banks(address_map &map);
	void x1_io_banks_common(address_map &map);
	void x1_mem(address_map &map);
	void x1turbo_io_banks(address_map &map);
	void x1turbo_mem(address_map &map);
protected:
	struct scrn_reg_t
	{
		uint8_t disp_bank;
		uint8_t pcg_mode;
		uint8_t v400_mode;
		uint8_t ank_sel;

		uint8_t pri;
		uint8_t blackclip; // x1 turbo specific
	};

	struct turbo_reg_t
	{
		uint8_t pal;
		uint8_t gfx_pal;
		uint8_t txt_pal[8];
		uint8_t txt_disp;
	};

	struct x1_rtc_t
	{
		uint8_t sec, min, hour, day, wday, month, year;
	};

	void x1_draw_pixel(bitmap_rgb32 &bitmap,int y,int x,uint16_t pen,uint8_t width,uint8_t height);
	void draw_fgtilemap(bitmap_rgb32 &bitmap,const rectangle &cliprect);
	void draw_gfxbitmap(bitmap_rgb32 &bitmap,const rectangle &cliprect, int plane,int pri);
	uint8_t check_prev_height(int x,int y,int x_size);
	uint8_t check_line_valid_height(int y,int x_size,int height);

	int priority_mixer_pri(int color);
	void cmt_command( uint8_t cmd );
	uint16_t jis_convert(int kanji_addr);

	required_device<address_map_bank_device> m_iobank;
	optional_device<ym2151_device> m_ym; // turbo-only
	optional_ioport m_sound_sw; // turbo-only
	required_shared_ptr<uint8_t> m_tvram;   /**< Pointer for Text Video RAM */
	required_shared_ptr<uint8_t> m_avram;   /**< Pointer for Attribute Video RAM */
	optional_shared_ptr<uint8_t> m_kvram;   /**< Pointer for Extended Kanji Video RAM (X1 Turbo) */
	required_memory_bank m_bitmapbank;
	required_region_ptr<uint8_t> m_ipl_rom;       /**< Pointer for IPL ROM */
	std::unique_ptr<uint8_t[]> m_work_ram;      /**< Pointer for base work RAM */
	std::unique_ptr<uint8_t[]> m_emm_ram;       /**< Pointer for EMM RAM */
	std::unique_ptr<uint8_t[]> m_pcg_ram;       /**< Pointer for PCG GFX RAM */
	required_region_ptr<uint8_t> m_cg_rom;        /**< Pointer for GFX ROM */
	required_region_ptr<uint8_t> m_kanji_rom;     /**< Pointer for Kanji ROMs */
	int m_xstart,           /**< Start X offset for screen drawing. */
		m_ystart;           /**< Start Y offset for screen drawing. */
	uint8_t m_hres_320;       /**< Pixel clock divider setting: (1) 48 (0) 24 */
	uint8_t m_io_switch;      /**< Enable access for special bitmap RMW phase in isolated i/o. */
	uint8_t m_io_sys;         /**< Read-back for PPI port C */
	uint8_t m_vsync;          /**< Screen V-Sync bit, active low */
	uint8_t m_vdisp;          /**< Screen V-Disp bit, active high */
	std::unique_ptr<uint8_t[]> m_gfx_bitmap_ram;    /**< Pointer for bitmap layer RAM. */
	uint8_t m_pcg_reset;      /**< @todo Unused variable. */
	uint8_t m_sub_obf;        /**< MCU side: OBF flag active low, indicates that there are parameters in comm buffer. */
	uint8_t m_ctc_irq_flag;       /**< @todo Unused variable. */
	scrn_reg_t m_scrn_reg;      /**< Base Video Registers. */
	turbo_reg_t m_turbo_reg;    /**< Turbo Z Video Registers. */
	x1_rtc_t m_rtc;         /**< Struct for RTC related variables */
	emu_timer *m_rtc_timer;     /**< Pointer for RTC timer. */
	uint8_t m_pcg_write_addr;     /**< @todo Unused variable. */
	uint8_t m_sub_cmd;        /**< MCU side: current command issued from Main to Sub. */
	uint8_t m_sub_cmd_length;     /**< MCU side: number of parameters, in bytes. */
	uint8_t m_sub_val[8];     /**< MCU side: parameters buffer. */
	int m_sub_val_ptr;      /**< MCU side: index for parameter read-back */
	int m_key_i;            /**< MCU side: index for keyboard read-back during OBF phase. */
	uint8_t m_irq_vector;     /**< @todo Unused variable. */
	uint8_t m_cmt_current_cmd;    /**< MCU side: CMT command issued. */
	uint8_t m_cmt_test;       /**< MCU side: Tape BREAK status bit. */
	uint8_t m_rom_index[3];       /**< Current ROM address. */
	uint32_t m_kanji_offset;      /**< @todo Unused variable. */
	uint8_t m_bios_offset;        /**< @todo Unused variable. */
	uint8_t m_x_b;            /**< Palette Register for Blue Gun */
	uint8_t m_x_g;            /**< Palette Register for Green Gun */
	uint8_t m_x_r;            /**< Palette Register for Red Gun */
	uint16_t m_kanji_addr_latch;  /**< Internal Kanji ROM address. */
	uint32_t m_kanji_addr;        /**< Latched Kanji ROM address. */
	uint8_t m_kanji_eksel;        /**< Kanji ROM register bit for latch phase. */
	uint8_t m_pcg_reset_occurred; /**< @todo Unused variable. */
	uint32_t m_old_key1;      /**< Keyboard read buffer for i/o port "key1" */
	uint32_t m_old_key2;      /**< Keyboard read buffer for i/o port "key2" */
	uint32_t m_old_key3;      /**< Keyboard read buffer for i/o port "key3" */
	uint32_t m_old_key4;      /**< Keyboard read buffer for i/o port "tenkey" */
	uint32_t m_old_fkey;      /**< Keyboard read buffer for i/o port "f_keys" */
	uint32_t m_emm_addr;      /**< EMM RAM current address */
	std::unique_ptr<uint8_t[]> m_pal_4096;      /**< X1 Turbo Z: pointer for 4096 palette entries */
	uint8_t m_crtc_vreg[0x100],   /**< CRTC register buffer. */
			m_crtc_index;       /**< CRTC register index. */
	uint8_t m_is_turbo;       /**< Machine type: (0) X1 Vanilla, (1) X1 Turbo */
	uint8_t m_ex_bank;        /**< X1 Turbo Z: RAM bank register */
	uint8_t m_ram_bank;       /**< Regular RAM bank for 0x0000-0x7fff memory window: (0) ROM/IPL (1) RAM */
	/**
	@brief Refresh current bitmap palette.
	*/
	void set_current_palette();
	/**
	@brief Retrieves the current PCG address.

	@param width Number of currently setted up CRTC characters
	@param y_char_size Number of scanlines per character.
	@return Destination PCG address.
	*/
	uint16_t get_pcg_addr(uint16_t width, uint8_t y_char_size);
	/**
	@brief X1 Turbo: Retrieves the current CHR ROM address in Hi-Speed Mode.

	@return Destination CHR address.
	*/
	uint16_t check_chr_addr();
	/**
	@brief X1 Turbo: Retrieves the current PCG ROM address in Hi-Speed Mode.

	@return Destination CHR address.
	*/
	uint16_t check_pcg_addr();
	/**
	@brief MCU side: retrieve keycode to game key conversion.

	@param port Address to convert.
	@return The converted game key buffer
	*/
	uint8_t get_game_key(uint8_t port);
	/**
	@brief MCU side: retrieve keyboard special key register.

	@return
	x--- ---- TEN: Numpad, Function key, special input key
	-x-- ---- KIN: Valid key
	--x- ---- REP: Key repeat
	---x ---- GRAPH key ON
	---- x--- CAPS lock ON
	---- -x-- KANA lock ON
	---- --x- SHIFT ON
	---- ---x CTRL ON
	*/
	uint8_t check_keyboard_shift();
	/**
	@brief convert MAME input to raw scancode for keyboard.

	@return the converted scancode
	@todo Unoptimized.
	*/
	uint16_t check_keyboard_press();

	uint8_t m_fdc_ctrl;

};

/*----------- defined in machine/x1.c -----------*/

DECLARE_DEVICE_TYPE(X1_KEYBOARD, x1_keyboard_device)

#endif // MAME_INCLUDES_X1_H
