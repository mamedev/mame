/*****************************************************************************
 *
 * includes/a7800.h
 *
 ****************************************************************************/

#ifndef A7800_H_
#define A7800_H_

#include "machine/6532riot.h"
#include "sound/pokey.h"
#include "sound/tiasound.h"
#include "sound/tiaintf.h"


class a7800_state : public driver_device
{
public:
	a7800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pokey(*this, "pokey"),
		m_tia(*this, "tia"),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_io_joysticks(*this, "joysticks"),
		m_io_buttons(*this, "buttons"),
		m_io_vblank(*this, "vblank"),
		m_io_console_buttons(*this, "console_buttons"),
		m_bank10(NULL),
		m_bank11(NULL),
		m_screen(*this, "screen") { }

	int m_lines;
	int m_ispal;
	unsigned char *m_cart_bkup;
	unsigned char *m_bios_bkup;
	int m_ctrl_lock;
	int m_ctrl_reg;
	int m_maria_flag;
	unsigned char *m_cartridge_rom;
	UINT16 m_cart_type;
	UINT32 m_cart_size;
	unsigned char m_stick_type;
	UINT8 *m_ROM;
	int m_maria_palette[32];
	int m_line_ram[2][160];
	int m_active_buffer;
	int m_maria_write_mode;
	unsigned int m_maria_dll;
	unsigned int m_maria_dl;
	int m_maria_holey;
	int m_maria_offset;
	int m_maria_vblank;
	int m_maria_dli;
	int m_maria_dmaon;
	int m_maria_dpp;
	int m_maria_wsync;
	int m_maria_backcolor;
	int m_maria_color_kill;
	int m_maria_cwidth;
	int m_maria_bcntl;
	int m_maria_kangaroo;
	int m_maria_rm;
	int m_maria_nmi;
	unsigned int m_maria_charbase;
	bitmap_ind16 m_bitmap;
	int m_p1_one_button;
	int m_p2_one_button;

	DECLARE_WRITE8_MEMBER(a7800_RAM0_w);
	DECLARE_WRITE8_MEMBER(a7800_cart_w);
	DECLARE_READ8_MEMBER(a7800_TIA_r);
	DECLARE_WRITE8_MEMBER(a7800_TIA_w);
	DECLARE_READ8_MEMBER(a7800_MARIA_r);
	DECLARE_WRITE8_MEMBER(a7800_MARIA_w);
	void a7800_driver_init(int ispal, int lines);
	DECLARE_DRIVER_INIT(a7800_pal);
	DECLARE_DRIVER_INIT(a7800_ntsc);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(a7800);
	DECLARE_PALETTE_INIT(a7800p);
	UINT32 screen_update_a7800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(a7800_interrupt);
	TIMER_CALLBACK_MEMBER(a7800_maria_startdma);
	DECLARE_READ8_MEMBER(riot_joystick_r);
	DECLARE_READ8_MEMBER(riot_console_button_r);
	DECLARE_WRITE8_MEMBER(riot_button_pullup_w);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( a7800_cart );

protected:
	required_device<cpu_device> m_maincpu;
	required_device<pokey_device> m_pokey;
	required_device<tia_device> m_tia;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_ioport m_io_joysticks;
	required_ioport m_io_buttons;
	required_ioport m_io_vblank;
	required_ioport m_io_console_buttons;
	memory_bank *m_bank10;
	memory_bank *m_bank11;
	required_device<screen_device> m_screen;

	void maria_draw_scanline();
	int is_holey(unsigned int addr);
	int write_line_ram(int addr, UINT8 offset, int pal);
	int a7800_verify_cart(char header[128]);
	UINT16 a7800_get_pcb_id(const char *pcb);
};

/*----------- defined in machine/a7800.c -----------*/

extern const riot6532_interface a7800_r6532_interface;

void a7800_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);

#endif /* A7800_H_ */
