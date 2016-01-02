// license:BSD3
// copyright-holders:Miso Kim
/*****************************************************************************
 *
 * includes/spc1500.h
 *
 ****************************************************************************/

#ifndef SPC1500_H_
#define SPC1500_H_

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "cpu/z80/z80daisy.h"
//#include "machine/z80ctc.h"
//#include "machine/z80dart.h"
#include "machine/i8255.h"
//#include "machine/wd_fdc.h"
//#include "machine/z80dma.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "formats/spc1000_tap.h"

struct scrn_reg_t
{
	UINT8 gfx_bank;
	UINT8 disp_bank;
	UINT8 pcg_mode;
	UINT8 v400_mode;
	UINT8 ank_sel;

	UINT8 pri;
//	UINT8 blackclip; // x1 turbo specific
};

struct turbo_reg_t
{
	UINT8 pal;
	UINT8 gfx_pal;
	UINT8 txt_pal[8];
	UINT8 txt_disp;
};

class spc1500_state : public driver_device
{
public:
	spc1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "mc6845")
		, m_cass(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_p_videoram(*this, "videoram")
		, m_io_kb(*this, "LINE")
		, m_io_joy(*this, "JOY")
		, m_centronics(*this, "centronics")
		, m_pio(*this, "i8255")
	{ }

	
	DECLARE_WRITE8_MEMBER(bank0_rom);
	DECLARE_WRITE8_MEMBER(bank0_ram);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE_LINE_MEMBER( centronics_busy_w ) { m_centronics_busy = state; }
	DECLARE_READ8_MEMBER(mc6845_videoram_r);
	DECLARE_WRITE8_MEMBER(cass_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(paletb_w);
	DECLARE_WRITE8_MEMBER(paletr_w);
	DECLARE_WRITE8_MEMBER(paletg_w);
	DECLARE_WRITE8_MEMBER(priority_w);
	DECLARE_WRITE8_MEMBER(pcgg_w);
	DECLARE_WRITE8_MEMBER(pcgb_w);
	DECLARE_WRITE8_MEMBER(pcgr_w);
	DECLARE_READ8_MEMBER(pcgg_r);
	DECLARE_READ8_MEMBER(pcgb_r);
	DECLARE_READ8_MEMBER(pcgr_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(romsel_w);
	DECLARE_WRITE8_MEMBER(ramsel_w);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	UINT8 m_IPLK;
	UINT8 m_GMODE;
	UINT16 m_page;
	UINT8 *m_work_ram;
	attotime m_time;
	UINT8 m_crtc_vreg[0x100];
	bool m_centronics_busy;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<z80_device> m_maincpu;
	required_device<mc6845_device> m_vdg;
	required_device<cassette_image_device> m_cass;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_p_videoram;
	required_ioport_array<10> m_io_kb;
	required_ioport m_io_joy;
	required_device<centronics_device> m_centronics;
	required_device<i8255_device> m_pio;
	
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<mc6845_device> m_crtc;
	required_device<z80ctc_device> m_ctc;

	std::unique_ptr<UINT8[]> m_tvram;         /**< Pointer for Text Video RAM */
	std::unique_ptr<UINT8[]> m_avram;         /**< Pointer for Attribute Video RAM */
	std::unique_ptr<UINT8[]> m_kvram;         /**< Pointer for Extended Kanji Video RAM (X1 Turbo) */
	UINT8 *m_ipl_rom;       /**< Pointer for IPL ROM */
	std::unique_ptr<UINT8[]> m_work_ram;      /**< Pointer for base work RAM */
	std::unique_ptr<UINT8[]> m_emm_ram;       /**< Pointer for EMM RAM */
	std::unique_ptr<UINT8[]> m_pcg_ram;       /**< Pointer for PCG GFX RAM */
	UINT8 *m_cg_rom;        /**< Pointer for GFX ROM */
	UINT8 *m_kanji_rom;     /**< Pointer for Kanji ROMs */
	int m_xstart,           /**< Start X offset for screen drawing. */
		m_ystart;           /**< Start Y offset for screen drawing. */
	UINT8 m_hres_320;       /**< Pixel clock divider setting: (1) 48 (0) 24 */
	UINT8 m_io_switch;      /**< Enable access for special bitmap RMW phase in isolated i/o. */
	UINT8 m_io_sys;         /**< Read-back for PPI port C */
	UINT8 m_vsync;          /**< Screen V-Sync bit, active low */
	UINT8 m_vdisp;          /**< Screen V-Disp bit, active high */
	UINT8 m_io_bank_mode;       /**< Helper for special bitmap RMW phase. */
	std::unique_ptr<UINT8[]> m_gfx_bitmap_ram;    /**< Pointer for bitmap layer RAM. */
	UINT8 m_pcg_reset;      /**< @todo Unused variable. */
	UINT8 m_sub_obf;        /**< MCU side: OBF flag active low, indicates that there are parameters in comm buffer. */
	UINT8 m_ctc_irq_flag;       /**< @todo Unused variable. */
	scrn_reg_t m_scrn_reg;      /**< Base Video Registers. */
	turbo_reg_t m_turbo_reg;    /**< Turbo Z Video Registers. */
	spc1500_rtc_t m_rtc;         /**< Struct for RTC related variables */
	emu_timer *m_rtc_timer;     /**< Pointer for RTC timer. */
	UINT8 m_pcg_write_addr;     /**< @todo Unused variable. */
	UINT8 m_sub_cmd;        /**< MCU side: current command issued from Main to Sub. */
	UINT8 m_sub_cmd_length;     /**< MCU side: number of parameters, in bytes. */
	UINT8 m_sub_val[8];     /**< MCU side: parameters buffer. */
	int m_sub_val_ptr;      /**< MCU side: index for parameter read-back */
	int m_key_i;            /**< MCU side: index for keyboard read-back during OBF phase. */
	UINT8 m_irq_vector;     /**< @todo Unused variable. */
	UINT8 m_cmt_current_cmd;    /**< MCU side: CMT command issued. */
	UINT8 m_cmt_test;       /**< MCU side: Tape BREAK status bit. */
	UINT8 m_rom_index[3];       /**< Current ROM address. */
	UINT32 m_kanji_offset;      /**< @todo Unused variable. */
	UINT8 m_bios_offset;        /**< @todo Unused variable. */
	UINT8 m_x_b;            /**< Palette Register for Blue Gun */
	UINT8 m_x_g;            /**< Palette Register for Green Gun */
	UINT8 m_x_r;            /**< Palette Register for Red Gun */
	UINT16 m_kanji_addr_latch;  /**< Internal Kanji ROM address. */
	UINT32 m_kanji_addr;        /**< Latched Kanji ROM address. */
	UINT8 m_kanji_eksel;        /**< Kanji ROM register bit for latch phase. */
	UINT8 m_pcg_reset_occurred; /**< @todo Unused variable. */
	UINT32 m_old_key1;      /**< Keyboard read buffer for i/o port "key1" */
	UINT32 m_old_key2;      /**< Keyboard read buffer for i/o port "key2" */
	UINT32 m_old_key3;      /**< Keyboard read buffer for i/o port "key3" */
	UINT32 m_old_key4;      /**< Keyboard read buffer for i/o port "tenkey" */
	UINT32 m_old_fkey;      /**< Keyboard read buffer for i/o port "f_keys" */
	UINT8 m_key_irq_flag;       /**< Keyboard IRQ pending. */
	UINT8 m_key_irq_vector;     /**< Keyboard IRQ vector. */
	UINT32 m_emm_addr;      /**< EMM RAM current address */
	std::unique_ptr<UINT8[]> m_pal_4096;      /**< X1 Turbo Z: pointer for 4096 palette entries */
	UINT8 m_crtc_vreg[0x100],   /**< CRTC register buffer. */
			m_crtc_index;       /**< CRTC register index. */
	UINT8 m_is_turbo;       /**< Machine type: (0) X1 Vanilla, (1) X1 Turbo */
	UINT8 m_ex_bank;        /**< X1 Turbo Z: RAM bank register */
	UINT8 m_ram_bank;       /**< Regular RAM bank for 0x0000-0x7fff memory window: (0) ROM/IPL (1) RAM */
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
	UINT16 get_pcg_addr(UINT16 width, UINT8 y_char_size);
	/**
	@brief X1 Turbo: Retrieves the current CHR ROM address in Hi-Speed Mode.

	@return Destination CHR address.
	*/
	UINT16 check_chr_addr();
	/**
	@brief X1 Turbo: Retrieves the current PCG ROM address in Hi-Speed Mode.

	@return Destination CHR address.
	*/
	UINT16 check_pcg_addr();
	/**
	@brief MCU side: retrieve keycode to game key conversion.

	@param port Address to convert.
	@return The converted game key buffer
	*/
	UINT8 get_game_key(UINT8 port);
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
	UINT8 check_keyboard_shift();
	/**
	@brief convert MAME input to raw scancode for keyboard.

	@return the converted scancode
	@todo Unoptimized.
	*/
	UINT16 check_keyboard_press();

	DECLARE_READ8_MEMBER(spc1500_mem_r);
	DECLARE_WRITE8_MEMBER(spc1500_mem_w);
	DECLARE_READ8_MEMBER(spc1500_io_r);
	DECLARE_WRITE8_MEMBER(spc1500_io_w);
	DECLARE_READ8_MEMBER(spc1500_sub_io_r);
	DECLARE_WRITE8_MEMBER(spc1500_sub_io_w);
	DECLARE_READ8_MEMBER(spc1500_rom_r);
	DECLARE_WRITE8_MEMBER(spc1500_rom_w);
	DECLARE_WRITE8_MEMBER(spc1500_rom_bank_0_w);
	DECLARE_WRITE8_MEMBER(spc1500_rom_bank_1_w);
	DECLARE_READ8_MEMBER(spc1500_fdc_r);
	DECLARE_WRITE8_MEMBER(spc1500_fdc_w);
	DECLARE_READ8_MEMBER(spc1500_pcg_r);
	DECLARE_WRITE8_MEMBER(spc1500_pcg_w);
	DECLARE_WRITE8_MEMBER(spc1500_pal_r_w);
	DECLARE_WRITE8_MEMBER(spc1500_pal_g_w);
	DECLARE_WRITE8_MEMBER(spc1500_pal_b_w);
	DECLARE_WRITE8_MEMBER(spc1500_ex_gfxram_w);
	DECLARE_WRITE8_MEMBER(spc1500_scrn_w);
	DECLARE_WRITE8_MEMBER(spc1500_pri_w);
	DECLARE_WRITE8_MEMBER(spc1500_6845_w);
	DECLARE_READ8_MEMBER(spc1500_kanji_r);
	DECLARE_WRITE8_MEMBER(spc1500_kanji_w);
	DECLARE_READ8_MEMBER(spc1500_emm_r);
	DECLARE_WRITE8_MEMBER(spc1500_emm_w);
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
	DECLARE_READ8_MEMBER(x1turbo_io_r);
	DECLARE_WRITE8_MEMBER(x1turbo_io_w);
	DECLARE_WRITE8_MEMBER(x1turboz_4096_palette_w);
	DECLARE_READ8_MEMBER(x1turboz_blackclip_r);
	DECLARE_READ8_MEMBER(x1turbo_bank_r);
	DECLARE_WRITE8_MEMBER(x1turbo_bank_w);
	DECLARE_READ8_MEMBER(spc1500_porta_r);
	DECLARE_READ8_MEMBER(spc1500_portb_r);
	DECLARE_READ8_MEMBER(spc1500_portc_r);
	DECLARE_WRITE8_MEMBER(spc1500_porta_w);
	DECLARE_WRITE8_MEMBER(spc1500_portb_w);
	DECLARE_WRITE8_MEMBER(spc1500_portc_w);
	DECLARE_DRIVER_INIT(spc1500_kanji);
	DECLARE_MACHINE_START(x1);
	DECLARE_MACHINE_RESET(x1);
	DECLARE_VIDEO_START(x1);
	DECLARE_PALETTE_INIT(x1);
	DECLARE_MACHINE_RESET(x1turbo);
	UINT32 screen_update_x1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_INPUT_CHANGED_MEMBER(ipl_reset);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset);
	TIMER_CALLBACK_MEMBER(spc1500_rtc_increment);
	TIMER_DEVICE_CALLBACK_MEMBER(spc1500_cmt_wind_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(spc1500_keyboard_callback);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	void spc1500_draw_pixel(bitmap_rgb32 &bitmap,int y,int x,UINT16 pen,UINT8 width,UINT8 height);
	void draw_fgtilemap(bitmap_rgb32 &bitmap,const rectangle &cliprect);
	void draw_gfxbitmap(bitmap_rgb32 &bitmap,const rectangle &cliprect, int plane,int pri);
	UINT8 check_prev_height(int x,int y,int x_size);
	UINT8 check_line_valid_height(int y,int x_size,int height);

	int priority_mixer_pri(int color);
	void cmt_command( UINT8 cmd );
	UINT16 jis_convert(int kanji_addr);

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<z80dma_device> m_dma;
};

/*----------- defined in machine/x1.c -----------*/

extern const device_type spc1500_KEYBOARD;

#endif /* SPC1500_H_ */
