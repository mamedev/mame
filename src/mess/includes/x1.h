/*****************************************************************************
 *
 * includes/x1.h
 *
 ****************************************************************************/

#ifndef X1_H_
#define X1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
//#include "machine/z80sio.h"
#include "machine/z80dart.h"
#include "machine/i8255.h"
#include "machine/wd17xx.h"
#include "machine/z80dma.h"
#include "video/mc6845.h"
#include "sound/2151intf.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cartslot.h"
#include "formats/basicdsk.h"
#include "formats/x1_tap.h"


// ======================> x1_keyboard_device

class x1_keyboard_device :	public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	x1_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	virtual void device_start();
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();
};

struct scrn_reg_t
{
	UINT8 gfx_bank;
	UINT8 disp_bank;
	UINT8 pcg_mode;
	UINT8 v400_mode;
	UINT8 ank_sel;

	UINT8 pri;
	UINT8 blackclip; // x1 turbo specific
};

struct turbo_reg_t
{
	UINT8 pal;
	UINT8 gfx_pal;
	UINT8 txt_pal[8];
	UINT8 txt_disp;
};

struct x1_rtc_t
{
	UINT8 sec, min, hour, day, wday, month, year;
};


class x1_state : public driver_device
{
public:
	x1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_x1_cpu(*this,"x1_cpu"),
	m_cass(*this, CASSETTE_TAG),
	m_fdc(*this, "fdc"),
	m_crtc(*this, "crtc"),
	m_ctc(*this, "ctc")
	{ }

	DECLARE_READ8_MEMBER(x1_mem_r);
	DECLARE_WRITE8_MEMBER(x1_mem_w);
	DECLARE_READ8_MEMBER(x1_io_r);
	DECLARE_WRITE8_MEMBER(x1_io_w);
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
	DECLARE_READ8_MEMBER(x1turbo_io_r);
	DECLARE_WRITE8_MEMBER(x1turbo_io_w);
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

	UINT8 *m_tvram;
	UINT8 *m_avram;
	UINT8 *m_kvram;
	UINT8 m_hres_320;
	UINT8 m_io_switch;
	UINT8 m_io_sys;
	UINT8 m_vsync;
	UINT8 m_vdisp;
	UINT8 m_io_bank_mode;
	UINT8 *m_gfx_bitmap_ram;
	UINT8 m_pcg_reset;
	UINT8 m_sub_obf;
	UINT8 m_ctc_irq_flag;
	scrn_reg_t m_scrn_reg;
	turbo_reg_t m_turbo_reg;
	x1_rtc_t m_rtc;
	emu_timer *m_rtc_timer;
	UINT8 m_pcg_write_addr;
	UINT8 m_sub_cmd;
	UINT8 m_sub_cmd_length;
	UINT8 m_sub_val[8];
	int m_sub_val_ptr;
	int m_key_i;
	UINT8 m_irq_vector;
	UINT8 m_cmt_current_cmd;
	UINT8 m_cmt_test;
	UINT8 m_rom_index[3];
	UINT32 m_kanji_offset;
	UINT8 m_bios_offset;
	UINT8 m_x_b;
	UINT8 m_x_g;
	UINT8 m_x_r;
	UINT16 m_kanji_addr_latch;
	UINT32 m_kanji_addr;
	UINT8 m_kanji_eksel;
	UINT8 m_pcg_reset_occurred;
	UINT32 m_old_key1;
	UINT32 m_old_key2;
	UINT32 m_old_key3;
	UINT32 m_old_key4;
	UINT32 m_old_fkey;
	UINT8 m_key_irq_flag;
	UINT8 m_key_irq_vector;
	UINT32 m_emm_addr;
	UINT8 *m_pal_4096;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;
	UINT8 m_is_turbo;
	UINT8 m_ex_bank;
	UINT8 m_ram_bank;
	void set_current_palette();
	UINT16 get_pcg_addr(UINT16 width, UINT8 y_char_size);
	UINT16 check_chr_addr();
	UINT16 check_pcg_addr();
	UINT8 get_game_key(UINT8 port);
	UINT8 check_keyboard_shift();
	UINT16 check_keyboard_press();

	required_device<cpu_device> m_x1_cpu;
	required_device<cassette_image_device> m_cass;
	required_device<device_t> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<z80ctc_device> m_ctc;
	DECLARE_DRIVER_INIT(x1_kanji);
	DECLARE_MACHINE_START(x1);
	DECLARE_MACHINE_RESET(x1);
	DECLARE_VIDEO_START(x1);
	DECLARE_PALETTE_INIT(x1);
	DECLARE_MACHINE_RESET(x1turbo);
	UINT32 screen_update_x1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_INPUT_CHANGED_MEMBER(ipl_reset);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset);
	TIMER_CALLBACK_MEMBER(x1_rtc_increment);
	TIMER_DEVICE_CALLBACK_MEMBER(x1_cmt_wind_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(x1_keyboard_callback);
};

/*----------- defined in machine/x1.c -----------*/

extern const device_type X1_KEYBOARD;

#endif /* X1_H_ */
